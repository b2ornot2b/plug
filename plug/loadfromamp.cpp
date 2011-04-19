#include "loadfromamp.h"
#include "ui_loadfromamp.h"

#include "mainwindow.h"

LoadFromAmp::LoadFromAmp(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::LoadFromAmp)
{
    ui->setupUi(this);

    QSettings settings;
    restoreGeometry(settings.value("Windows/loadAmpPresetWindowGeometry").toByteArray());

    connect(ui->pushButton, SIGNAL(clicked()), this, SLOT(load()));
    connect(ui->pushButton_2, SIGNAL(clicked()), this, SLOT(close()));
}

LoadFromAmp::~LoadFromAmp()
{
    QSettings settings;
    settings.setValue("Windows/loadAmpPresetWindowGeometry", saveGeometry());
    delete ui;
}

void LoadFromAmp::load()
{
    QSettings settings;

    ((MainWindow*)parent())->load_from_amp(ui->comboBox->currentIndex());
    ((MainWindow*)parent())->set_index(ui->comboBox->currentIndex());
    if(!settings.value("Settings/keepWindowsOpen").toBool())
        this->close();
}

void LoadFromAmp::load_names(char names[][32])
{
    for(int i = 0; i < 100; i++)
    {
        if(names[i][0] == 0x00)
            break;
        ui->comboBox->addItem(QString("[%1] %2").arg(i+1).arg(names[i]));
    }
}

void LoadFromAmp::delete_items()
{
    int j = ui->comboBox->count();
    for(int i = 0; i < j; i++)
        ui->comboBox->removeItem(0);
}

void LoadFromAmp::change_name(int slot, QString *name)
{
    ui->comboBox->setItemText(slot, *name);
    ui->comboBox->setCurrentIndex(slot);
}
