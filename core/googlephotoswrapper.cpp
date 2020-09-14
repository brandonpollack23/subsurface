#include "googlephotoswrapper.h"

#include "api_keys.h"
#include "core/settings/qPrefMedia.h"

#include <QDesktopServices>
#include <QtNetworkAuth>

GooglePhotosWrapper *GooglePhotosWrapper::m_instance = NULL;

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
	});

	connect(&oauth2, &QOAuth2AuthorizationCodeFlow::requestFailed, [=]() {
		qDebug() << __FUNCTION__ << __LINE__ << "Google OAuth2 Access failed!";
		emit authenticationFailed();
	});

	auto media = qPrefMedia::instance();
	if (!media->google_photos_token().isEmpty()) {
		oauth2.setToken(media->google_photos_token());
		// TODO NOW double check this works for granting again without going through auth flow.
		grantAuthentication();
	}
}

QNetworkReply *GooglePhotosWrapper::requestPhotosFromTimespan(QDateTime startDateTime, QDateTime endDateTime)
{
	return nullptr;
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