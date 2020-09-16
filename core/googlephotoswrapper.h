#ifndef GOOGLEPHOTOSWRAPPER_H
#define GOOGLEPHOTOSWRAPPER_H

#include <QtCore>
#include <QtNetwork>

#include <QOAuth2AuthorizationCodeFlow>

class GooglePhotosList
{
public:
	GooglePhotosList(QJsonArray photos);

	// TODO NOW
	// make iterator that autofetches continuations.
private:
	QJsonArray photos;
};

class GooglePhotosWrapper : public QObject
{
	Q_OBJECT

public:
	static GooglePhotosWrapper *instance();
	GooglePhotosWrapper(QObject *parent = nullptr);

	GooglePhotosList requestPhotosFromTimespan(QDateTime startDateTime, QDateTime endDateTime);

public slots:
	void grantAuthentication();

signals:
	void authenticated();
	void authenticationFailed();

	void replied();

public:
	bool isAuthenticated() const;
	const QString token() const;

private:
	void prepareRequest(QNetworkRequest *request, QUrl url);

private:
	static GooglePhotosWrapper *m_instance;
	QOAuth2AuthorizationCodeFlow oauth2;
};

#endif // REDDITWRAPPER_H
