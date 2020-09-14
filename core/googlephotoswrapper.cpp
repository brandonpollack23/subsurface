#include "googlephotoswrapper.h"

#include "api_keys.h"

#include <QDesktopServices>
#include <QtNetworkAuth>

GooglePhotosWrapper::GooglePhotosWrapper(QObject *parent) : QObject(parent)
{
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
}

QNetworkReply *GooglePhotosWrapper::requestPhotosFromTimespan(QDateTime startDateTime, QDateTime endDateTime)
{
	return nullptr;
}

void GooglePhotosWrapper::grantAuthentication()
{
	oauth2.grant();
}