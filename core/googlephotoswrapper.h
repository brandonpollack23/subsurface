#ifndef GOOGLEPHOTOSWRAPPER_H
#define GOOGLEPHOTOSWRAPPER_H

#include <QtCore>
#include <QtNetwork>

#include <QOAuth2AuthorizationCodeFlow>

class GooglePhotosWrapper : public QObject
{
	Q_OBJECT

public:
	GooglePhotosWrapper(QObject *parent = nullptr);

	QNetworkReply *requestPhotosFromTimespan(QDateTime startDateTime, QDateTime endDateTime);

public slots:
	void grantAuthentication();

signals:
	void authenticated();

private:
	QOAuth2AuthorizationCodeFlow oauth2;
};

#endif // REDDITWRAPPER_H
