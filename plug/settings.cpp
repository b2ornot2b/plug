#include "settings.h"
#include "ui_settings.h"

Settings::Settings(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Settings)
{
    QSettings settings;

    ui->setupUi(this);

    ui->checkBox->setChecked(settings.value("Settings/checkForUpdates").toBool());
    ui->checkBox_2->setChecked(settings.value("Settings/connectOnStartup").toBool());
    ui->checkBox_3->setChecked(settings.value("Settings/oneSetToSetThemAll").toBool());
    ui->checkBox_4->setChecked(settings.value("Settings/keepWindowsOpen").toBool());
    ui->checkBox_5->setChecked(settings.value("Settings/popupChangedWindows").toBool());

    connect(ui->checkBox, SIGNAL(toggled(bool)), this, SLOT(change_updates(bool)));
    connect(ui->checkBox_2, SIGNAL(toggled(bool)), this, SLOT(change_connect(bool)));
    connect(ui->checkBox_3, SIGNAL(toggled(bool)), this, SLOT(change_oneset(bool)));
    connect(ui->checkBox_4, SIGNAL(toggled(bool)), this, SLOT(change_keepopen(bool)));
    connect(ui->checkBox_5, SIGNAL(toggled(bool)), this, SLOT(change_popupwindows(bool)));
}

Settings::~Settings()
{
    delete ui;
}

void Settings::change_updates(bool value)
{
    QSettings settings;

    settings.setValue("Settings/checkForUpdates", value);
}

void Settings::change_connect(bool value)
{
    QSettings settings;

    settings.setValue("Settings/connectOnStartup", value);
}

void Settings::change_oneset(bool value)
{
    QSettings settings;

    settings.setValue("Settings/oneSetToSetThemAll", value);
}

void Settings::change_keepopen(bool value)
{
    QSettings settings;

    settings.setValue("Settings/keepWindowsOpen", value);
}

void Settings::change_popupwindows(bool value)
{
    QSettings settings;

    settings.setValue("Settings/popupChangedWindows", value);
}
