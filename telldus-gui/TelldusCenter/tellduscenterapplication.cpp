#include "tellduscenterapplication.h"
#include <QMessageBox>
#include <QDir>
#include <QIcon>
#include <QPluginLoader>

#include <QDebug>
#include <telldus-core.h>
#include "tellduscenterplugin.h"

TelldusCenterApplication::TelldusCenterApplication(int &argc, char **argv)
		:QApplication(argc, argv),
		systrayIcon(this)
{
	connect(&systrayIcon, SIGNAL(showEventMessage(const QString &, const QString &, const QString &)), this, SLOT(showMessage(QString,QString,QString)));

	setQuitOnLastWindowClosed( false );

	//We use queued connection since it is called from another thread
	connect(this, SIGNAL(sigDeviceEvent(int, int, const QString &)), this, SLOT(deviceEvent(int, int, const QString &)), Qt::QueuedConnection);
	tdInit();
	tdRegisterDeviceEvent( &TelldusCenterApplication::deviceEvent, 0 );

	loadPlugins();
}

TelldusCenterApplication::~TelldusCenterApplication() {
	qDeleteAll(p_plugins);
	tdClose();
}

PluginList TelldusCenterApplication::plugins() const {
	return p_plugins;
}

void TelldusCenterApplication::showMainWindow() {
	if (!isMainWindowShown()) {
		mainWindow = new MainWindow();
	}
	mainWindow->show();
}

bool TelldusCenterApplication::isMainWindowShown() {
	return !mainWindow.isNull();
}

#if defined(Q_WS_MAC)
bool TelldusCenterApplication::event(QEvent *event)
{
	switch (event->type()) {
	case QEvent::ApplicationActivate: {
		showMainWindow();
		return true;
	}
	default:
		break;
	}
	return QApplication::event(event);
}
#endif

void TelldusCenterApplication::eventTriggered( const QString &name, const QString &title ) {
	qDebug() << "Systray - eventTriggered:" << name << title;
}

void TelldusCenterApplication::showMessage( const QString &title, const QString &message, const QString &detailedMessage ) {
	if (isMainWindowShown()) {
		mainWindow->showMessage(title, message, detailedMessage);
	} else {
		systrayIcon.showMessage((title != "" ? title : "Telldus Center"), message, QSystemTrayIcon::Warning);
	}
}

void TelldusCenterApplication::deviceEvent(int deviceId, int method, const char *data, int /*callbackId*/, void */*context*/) {
	TelldusCenterApplication *app = TelldusCenterApplication::instance();
	emit app->sigDeviceEvent(deviceId, method, data);
}

void TelldusCenterApplication::loadPlugins() {
	QDir pluginsDir = QDir(qApp->applicationDirPath());

#if defined(Q_OS_WIN)
	if (pluginsDir.dirName().toLower() == "debug" || pluginsDir.dirName().toLower() == "release")
		pluginsDir.cdUp();
#elif defined(Q_OS_MAC)
	if (pluginsDir.dirName() == "MacOS") {
		pluginsDir.cdUp();
		pluginsDir.cdUp();
		pluginsDir.cdUp();
	}
#endif
	pluginsDir.cd("plugins");

	foreach (QString fileName, pluginsDir.entryList(QDir::Files)) {
		QPluginLoader loader(pluginsDir.absoluteFilePath(fileName));
		QObject *plugin = loader.instance();
		if (plugin) { //Load the plugin
			loadPlugin(plugin);
		}
	}
}

void TelldusCenterApplication::loadPlugin(QObject *plugin) {
	TelldusCenterPlugin *iPlugin = qobject_cast<TelldusCenterPlugin *>(plugin);
	if (!iPlugin) {
		return;
	}
	p_plugins.append(iPlugin);
}

void TelldusCenterApplication::deviceEvent(int deviceId, int method, const QString &/*data*/) {
	char *name = tdGetName(deviceId);
	QString deviceName(name);
	free(name);
	QString methodName;
	switch( method ) {
	case TELLSTICK_TURNON:
		methodName = "turned on";
		break;
	case TELLSTICK_TURNOFF:
		methodName = "turned off";
		break;
	case TELLSTICK_BELL:
		methodName = "belled";
		break;
	case TELLSTICK_DIM:
		methodName = "dimmed to level x%";
		break;
	}
	showMessage("", QString("%1 %2").arg(deviceName).arg(methodName), "");
}

TelldusCenterApplication *TelldusCenterApplication::instance() {
	return (static_cast<TelldusCenterApplication *>(QCoreApplication::instance()));
}