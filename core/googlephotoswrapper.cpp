#include "googlephotoswrapper.h"

#include "api_keys.h"
#include "core/settings/qPrefMedia.h"

#include <QDesktopServices>
#include <QtNetworkAuth>

const QString bearerFormat = QStringLiteral("Bearer %1"); // Case sensitive

GooglePhotosList::GooglePhotosList(QJsonArray photos) : photos(photos){};

GooglePhotosWrapper *GooglePhotosWrapper::m_instance = nullptr;

GooglePhotosWrapper *GooglePhotosWrapper::instance()
{
	if (!m_instance) {
		return new GooglePhotosWrapper();
	}
	return m_instance;
}

GooglePhotosWrapper::GooglePhotosWrapper(QObject *parent) : QObject(parent)
{
	if (m_instance) {
		// TODO instead of singleton, one per user.
		qDebug() << "trying to create an additional GooglePhotosWrapper object";
		return;
	}
	m_instance = this;

	auto replyHandler = new QOAuthHttpServerReplyHandler(5476, this);
	oauth2.setReplyHandler(replyHandler);

	oauth2.setAuthorizationUrl(QUrl("https://accounts.google.com/o/oauth2/auth"));
	oauth2.setAccessTokenUrl(QUrl("https://oauth2.googleapis.com/token"));
	oauth2.setClientIdentifier(GOOGLE_PHOTOS_CLIENT_ID);
	oauth2.setClientIdentifierSharedKey(GOOGLE_PHOTOS_API_KEY);

	oauth2.setScope("https://www.googleapis.com/auth/photoslibrary.readonly");

	connect(&oauth2, &QOAuth2AuthorizationCodeFlow::authorizeWithBrowser, &QDesktopServices::openUrl);

	connect(&oauth2, &QOAuth2AuthorizationCodeFlow::granted, [=]() {
		qDebug() << __FUNCTION__ << __LINE__ << "Google OAuth2 Access Granted!";
		emit authenticated();
		// TODO divelistview::loadWebImagesFromGooglePhotos
	});

	connect(&oauth2, &QOAuth2AuthorizationCodeFlow::requestFailed, [=]() {
		qDebug() << __FUNCTION__ << __LINE__ << "Google OAuth2 Access failed!";
		emit authenticationFailed();
	});

	auto media = qPrefMedia::instance();
	if (!media->google_photos_token().isEmpty()) {
		// TODO NOW remove
		qDebug() << "Google Photos Token found: " << media->google_photos_token();
		oauth2.setToken(media->google_photos_token());
		// TODO NOW double check this works for granting again without going through auth flow.
		grantAuthentication();
	}
}

QJsonObject convertToGoogleDate(QDateTime dt)
{
	QJsonObject result;
	// TODO NOW use input
	result["year"] = 2020;
	result["month"] = 9;
	result["day"] = 9;
	return result;
}

GooglePhotosList GooglePhotosWrapper::requestPhotosFromTimespan(QDateTime startDateTime, QDateTime endDateTime)
{
	qDebug() << "Requesting photos in timespan from google photos: " << startDateTime << " to " << endDateTime;

	if (!isAuthenticated()) {
		return GooglePhotosList(QJsonArray());
	}

	QJsonObject search_query = QJsonDocument::fromJson(
					   R"(
							 {
							  "filters": {
									 "dateFilter": {
										  "ranges": [ 
												{ 
													"startDate": { "year": 2020, "month": 9, "day": 9},
													"endDate": { "year": 2020, "month": 9, "day": 9} }
											]} 
								}
							})")
					   .object();

	QJsonArray filters = search_query["filters"].toArray();
	QJsonObject filter = filters[0].toObject();
	QJsonObject dateFilter = filter["dateFilter"].toObject();
	QJsonObject range = dateFilter["ranges"].toArray()[0].toObject();
	// range["startDate"] = convertToGoogleDate(startDateTime);
	// range["endDate"] = convertToGoogleDate(endDateTime);

	// TODO NOW remove
	qDebug() << "Requesting date range with query: " << search_query;

	QNetworkRequest request;
	prepareRequest(&request, QUrl("https://photoslibrary.googleapis.com/v1/mediaItems:search"));
	auto search_result_reply = oauth2.networkAccessManager()->post(request, QJsonDocument(search_query).toJson());

	// TODO NOW not blocking
	std::shared_ptr<QJsonArray> result;
	qDebug() << "Gross sync stuff, acquiring sem";
	QSemaphore sem(1);
	sem.acquire();
	connect(
		search_result_reply, &QNetworkReply::finished, [search_result_reply, result, &sem]() {
			search_result_reply->deleteLater();
			if (search_result_reply->error() != QNetworkReply::NoError) {
				// emit error(search_result_reply->errorString());
				// TODO NOW add and emit error
				qDebug() << "ERROR!!! " << search_result_reply->errorString();
				return;
			}

			const auto document = QJsonDocument::fromJson(search_result_reply->readAll());

			// TODO NOW remove
			qDebug() << "Responded to Google Photos query with: " << document;

			// emit timeSpanComplete(startDateTime, endDateTime, GooglePhotosList(document["mediaItems"].toArray()));
			*result = document["mediaItems"].toArray();
			qDebug() << "Gross sync stuff, done";
			sem.release();
		});

	// TODO NOW REMOVE
	connect(
		search_result_reply,
		QOverload<QNetworkReply::NetworkError>::of(&QNetworkReply::error),
		[search_result_reply](QNetworkReply::NetworkError error) {
			qDebug() << "ERROR!!! " << error << " " << search_result_reply->errorString();
		});

	qDebug() << "Gross sync stuff, waiting here";
	sem.acquire();

	// TODO NOW continuation iterator.
	return GooglePhotosList(*result);
}

void GooglePhotosWrapper::prepareRequest(QNetworkRequest *request, QUrl url)
{
	request->setUrl(url);
	request->setHeader(QNetworkRequest::UserAgentHeader, oauth2.userAgent());
	const QString bearer = bearerFormat.arg(oauth2.token());
	request->setRawHeader("Authorization", bearer.toUtf8());
}

void GooglePhotosWrapper::grantAuthentication()
{
	oauth2.grant();
}

bool GooglePhotosWrapper::isAuthenticated() const
{
	return oauth2.status() == QOAuth2AuthorizationCodeFlow::Status::Granted;
}

const QString GooglePhotosWrapper::token() const
{
	if (isAuthenticated()) {
		return oauth2.token();
	} else {
		return "";
	}
}