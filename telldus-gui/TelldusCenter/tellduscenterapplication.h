#ifndef TELLDUSCENTERAPPLICATION_H
#define TELLDUSCENTERAPPLICATION_H

#include <QApplication>
#include <QPointer>
#include <QScriptValue>

#include <telldus-core.h>

#include "mainwindow.h"

class TelldusCenterPlugin;
class TelldusCenterApplicationPrivate;

typedef QList<TelldusCenterPlugin *> PluginList;

class TelldusCenterApplication : public QApplication
{
	Q_OBJECT
public:
	TelldusCenterApplication(int &argc, char **argv);
	virtual ~TelldusCenterApplication();

#if defined(Q_WS_MAC)
	bool event(QEvent *event);
#endif

	PluginList plugins() const;

	void loadPlugins();
	void loadScripts();
	void loadToolbar();

	static TelldusCenterApplication *instance();
	

signals:
	void allDoneLoading();
	void showMessage( const QString &title, const QString &message, const QString &detailedMessage );

public slots:
	void addWidget( const QString &page, const QString &icon, QWidget *widget );
	void addWidget( const QString &page, const QIcon &icon, QWidget *widget );
	void eventTriggered( const QString &name, const QString &title );
	bool isMainWindowShown();
	QScriptValue mainWindow();
	void showMainWindow();

private slots:
	void scriptException( const QScriptValue &exception );

private:
	void loadPlugin(QObject *plugin);

	TelldusCenterApplicationPrivate *d;
};

#endif // TELLDUSCENTERAPPLICATION_H
