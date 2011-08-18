#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    memset(names, 0x00, 100*32);

    // load window size
    QSettings settings;
    restoreGeometry(settings.value("Windows/mainWindowGeometry").toByteArray());
    restoreState(settings.value("Windows/mainWindowState").toByteArray());

    // setting default values if there was none
    if(!settings.contains("Settings/checkForUpdates"))
        settings.setValue("Settings/checkForUpdates", true);
    if(!settings.contains("Settings/connectOnStartup"))
        settings.setValue("Settings/connectOnStartup", true);
    if(!settings.contains("Settings/oneSetToSetThemAll"))
        settings.setValue("Settings/oneSetToSetThemAll", false);
    if(!settings.contains("Settings/keepWindowsOpen"))
        settings.setValue("Settings/keepWindowsOpen", false);
    if(!settings.contains("Settings/popupChangedWindows"))
        settings.setValue("Settings/popupChangedWindows", true);
    if(!settings.contains("Settings/defaultEffectValues"))
        settings.setValue("Settings/defaultEffectValues", true);

    // create child objects
    amp_ops = new Mustang();
    amp = new Amplifier(this);
    effect1 = new Effect(this, 0);
    effect2 = new Effect(this, 1);
    effect3 = new Effect(this, 2);
    effect4 = new Effect(this, 3);

    about_window = new About(this);
    save = new SaveOnAmp(this);
    load = new LoadFromAmp(this);
    seffects = new SaveEffects(this);
    settings_win = new Settings(this);
    saver = new SaveToFile(this);
    quickpres = new QuickPresets(this);

    connected = false;

    // connect buttons to slots
    connect(ui->Amplifier, SIGNAL(clicked()), amp, SLOT(show()));
    connect(ui->EffectButton1, SIGNAL(clicked()), effect1, SLOT(show()));
    connect(ui->EffectButton2, SIGNAL(clicked()), effect2, SLOT(show()));
    connect(ui->EffectButton3, SIGNAL(clicked()), effect3, SLOT(show()));
    connect(ui->EffectButton4, SIGNAL(clicked()), effect4, SLOT(show()));
    connect(ui->actionConnect, SIGNAL(triggered()), this, SLOT(start_amp()));
    connect(ui->actionDisconnect, SIGNAL(triggered()), this, SLOT(stop_amp()));
    connect(ui->actionExit, SIGNAL(triggered()), this, SLOT(close()));
    connect(ui->actionAbout, SIGNAL(triggered()), about_window, SLOT(open()));
    connect(ui->actionSave_to_amplifier, SIGNAL(triggered()), save, SLOT(show()));
    connect(ui->action_Load_from_amplifier, SIGNAL(triggered()), load, SLOT(show()));
    connect(ui->actionSave_effects, SIGNAL(triggered()), seffects, SLOT(open()));
    connect(ui->actionCheck_for_Updates, SIGNAL(triggered()), this, SLOT(check_for_updates()));
    connect(ui->action_Options, SIGNAL(triggered()), settings_win, SLOT(show()));
    connect(ui->actionL_oad_from_file, SIGNAL(triggered()), this, SLOT(loadfile()));
    connect(ui->actionS_ave_to_file, SIGNAL(triggered()), saver, SLOT(show()));
    connect(ui->action_Library_view, SIGNAL(triggered()), this, SLOT(show_library()));
    connect(ui->action_Update_firmware, SIGNAL(triggered()), this, SLOT(update_firmware()));
    connect(ui->action_Default_effects, SIGNAL(triggered()), this, SLOT(show_default_effects()));
    connect(ui->action_Quick_presets, SIGNAL(triggered()), quickpres, SLOT(show()));

    // shortcuts to activate effect windows
    QShortcut *showfx1 = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_1), this, 0, 0, Qt::ApplicationShortcut);
    QShortcut *showfx2 = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_2), this, 0, 0, Qt::ApplicationShortcut);
    QShortcut *showfx3 = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_3), this, 0, 0, Qt::ApplicationShortcut);
    QShortcut *showfx4 = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_4), this, 0, 0, Qt::ApplicationShortcut);
    QShortcut *showamp = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_5), this, 0, 0, Qt::ApplicationShortcut);
    connect(showfx1, SIGNAL(activated()), this, SLOT(show_fx1()));
    connect(showfx2, SIGNAL(activated()), this, SLOT(show_fx2()));
    connect(showfx3, SIGNAL(activated()), this, SLOT(show_fx3()));
    connect(showfx4, SIGNAL(activated()), this, SLOT(show_fx4()));
    connect(showamp, SIGNAL(activated()), this, SLOT(show_amp()));

    // shortcuts for quick loading presets
    QShortcut *loadpres0 = new QShortcut(QKeySequence(Qt::Key_0), this, 0, 0, Qt::ApplicationShortcut);
    QShortcut *loadpres1 = new QShortcut(QKeySequence(Qt::Key_1), this, 0, 0, Qt::ApplicationShortcut);
    QShortcut *loadpres2 = new QShortcut(QKeySequence(Qt::Key_2), this, 0, 0, Qt::ApplicationShortcut);
    QShortcut *loadpres3 = new QShortcut(QKeySequence(Qt::Key_3), this, 0, 0, Qt::ApplicationShortcut);
    QShortcut *loadpres4 = new QShortcut(QKeySequence(Qt::Key_4), this, 0, 0, Qt::ApplicationShortcut);
    QShortcut *loadpres5 = new QShortcut(QKeySequence(Qt::Key_5), this, 0, 0, Qt::ApplicationShortcut);
    QShortcut *loadpres6 = new QShortcut(QKeySequence(Qt::Key_6), this, 0, 0, Qt::ApplicationShortcut);
    QShortcut *loadpres7 = new QShortcut(QKeySequence(Qt::Key_7), this, 0, 0, Qt::ApplicationShortcut);
    QShortcut *loadpres8 = new QShortcut(QKeySequence(Qt::Key_8), this, 0, 0, Qt::ApplicationShortcut);
    QShortcut *loadpres9 = new QShortcut(QKeySequence(Qt::Key_9), this, 0, 0, Qt::ApplicationShortcut);
    connect(loadpres0, SIGNAL(activated()), this, SLOT(load_presets0()));
    connect(loadpres1, SIGNAL(activated()), this, SLOT(load_presets1()));
    connect(loadpres2, SIGNAL(activated()), this, SLOT(load_presets2()));
    connect(loadpres3, SIGNAL(activated()), this, SLOT(load_presets3()));
    connect(loadpres4, SIGNAL(activated()), this, SLOT(load_presets4()));
    connect(loadpres5, SIGNAL(activated()), this, SLOT(load_presets5()));
    connect(loadpres6, SIGNAL(activated()), this, SLOT(load_presets6()));
    connect(loadpres7, SIGNAL(activated()), this, SLOT(load_presets7()));
    connect(loadpres8, SIGNAL(activated()), this, SLOT(load_presets8()));
    connect(loadpres9, SIGNAL(activated()), this, SLOT(load_presets9()));

    // shortcut to activate buttons
    QShortcut *shortcut = new QShortcut(QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_A), this);
    connect(shortcut, SIGNAL(activated()), this, SLOT(enable_buttons()));

    // connect the functions if needed
    if(settings.value("Settings/checkForUpdates").toBool())
        connect(this, SIGNAL(started()), this, SLOT(check_for_updates()));
    if(settings.value("Settings/connectOnStartup").toBool())
        connect(this, SIGNAL(started()), this, SLOT(start_amp()));

    this->show();
    this->repaint();

    emit started();
}

MainWindow::~MainWindow()
{
    QSettings settings;
    settings.setValue("Windows/mainWindowGeometry", saveGeometry());
    settings.setValue("Windows/mainWindowState", saveState());
    delete amp_ops;    // stop the communication before exiting
    delete ui;
}

void MainWindow::start_amp()
{
    QSettings settings;
    int x;
    struct amp_settings amplifier_set;
    struct fx_pedal_settings effects_set[4];
    char name[32];

    ui->statusBar->showMessage(tr("Connecting..."));
    this->repaint();  // this should not be needed!
    x = amp_ops->start_amp(names, name, &amplifier_set, effects_set);    // request initialization of communication

    if(x != 0)    // if request succeded
    {
        if(x == -100)
            ui->statusBar->showMessage(tr("Suitable device not found!"), 5000);
        else
            ui->statusBar->showMessage(QString(tr("Error: %1")).arg(x), 5000);
        return;
    }

    load->load_names(names);
    save->load_names(names);
    quickpres->load_names(names);

    if(name[0] == 0x00)
        setWindowTitle(QString(tr("PLUG: NONE")));
    else
        setWindowTitle(QString(tr("PLUG: %1")).arg(name));

    current_name = name;

    amp->load(amplifier_set);
    if(settings.value("Settings/popupChangedWindows").toBool())
        amp->show();
    for(int i = 0; i < 4; i++)
    {
        switch(effects_set[i].fx_slot)
        {
        case 0x00:
        case 0x04:
            effect1->load(effects_set[i]);
            if(effects_set[i].effect_num)
                if(settings.value("Settings/popupChangedWindows").toBool())
                    effect1->show();
            break;

        case 0x01:
        case 0x05:
            effect2->load(effects_set[i]);
            if(effects_set[i].effect_num)
                if(settings.value("Settings/popupChangedWindows").toBool())
                    effect2->show();
            break;

        case 0x02:
        case 0x06:
            effect3->load(effects_set[i]);
            if(effects_set[i].effect_num)
                if(settings.value("Settings/popupChangedWindows").toBool())
                    effect3->show();
            break;

        case 0x03:
        case 0x07:
            effect4->load(effects_set[i]);
            if(effects_set[i].effect_num)
                if(settings.value("Settings/popupChangedWindows").toBool())
                    effect4->show();
            break;
        }
    }

    // activate buttons
    amp->enable_set_button(true);
    effect1->enable_set_button(true);
    effect2->enable_set_button(true);
    effect3->enable_set_button(true);
    effect4->enable_set_button(true);
    ui->actionConnect->setDisabled(true);
    ui->actionDisconnect->setDisabled(false);
    ui->actionSave_to_amplifier->setDisabled(false);
    ui->action_Load_from_amplifier->setDisabled(false);
    ui->actionSave_effects->setDisabled(false);
    ui->action_Library_view->setDisabled(false);
    ui->statusBar->showMessage(tr("Connected"), 3000);    // show message on the status bar

    connected = true;
}

void MainWindow::stop_amp()
{
    int x;

    save->delete_items();
    load->delete_items();
    quickpres->delete_items();

    x = amp_ops->stop_amp();
    if(x == 0)    // if request succeded
    {
        // deactivate buttons
        amp->enable_set_button(false);
        effect1->enable_set_button(false);
        effect2->enable_set_button(false);
        effect3->enable_set_button(false);
        effect4->enable_set_button(false);
        ui->actionConnect->setDisabled(false);
        ui->actionDisconnect->setDisabled(true);
        ui->actionSave_to_amplifier->setDisabled(true);
        ui->action_Load_from_amplifier->setDisabled(true);
        ui->actionSave_effects->setDisabled(true);
        ui->action_Library_view->setDisabled(true);
        setWindowTitle(QString(tr("PLUG")));
        ui->statusBar->showMessage(tr("Disconnected"), 5000);    // show message on the status bar

        connected = false;
    }
    else    // if request failed
        ui->statusBar->showMessage(QString(tr("Error: %1")).arg(x), 5000);
}

// pass the message to the amp
int MainWindow::set_effect(struct fx_pedal_settings pedal)
{
    QSettings settings;

    if(!connected)
        return 0;

    if(!settings.value("Settings/oneSetToSetThemAll").toBool())
        return amp_ops->set_effect(pedal);
    amp->send_amp();
    return 0;
}

int MainWindow::set_amplifier(struct amp_settings amp_settings)
{
    QSettings settings;

    if(!connected)
        return 0;

    if(settings.value("Settings/oneSetToSetThemAll").toBool())
    {
        struct fx_pedal_settings pedal;

        if(effect1->get_changed())
        {
            effect1->get_settings(pedal);
            amp_ops->set_effect(pedal);
        }
        if(effect2->get_changed())
        {
            effect2->get_settings(pedal);
            amp_ops->set_effect(pedal);
        }
        if(effect3->get_changed())
        {
            effect3->get_settings(pedal);
            amp_ops->set_effect(pedal);
        }
        if(effect4->get_changed())
        {
            effect4->get_settings(pedal);
            amp_ops->set_effect(pedal);
        }
    }

    return amp_ops->set_amplifier(amp_settings);
}

int MainWindow::save_on_amp(char *name, int slot)
{
    int ret;

    if(!connected)
        return -1;

    ret = amp_ops->save_on_amp(name, slot);

    if(name[0] == 0x00)
        setWindowTitle(QString(tr("PLUG: NONE")));
    else
        setWindowTitle(QString(tr("PLUG: %1")).arg(name));

    current_name=name;
    memcpy(names[slot], name, 32);

    return ret;
}

int MainWindow::load_from_amp(int slot)
{
    QSettings settings;
    struct amp_settings amplifier_set;
    struct fx_pedal_settings effects_set[4];
    char name[32];

    if(!connected)
        return -1;

    amp_ops->load_memory_bank(slot, name, &amplifier_set, effects_set);

    if(name[0] == 0x00)
        setWindowTitle(QString(tr("PLUG: NONE")));
    else
        setWindowTitle(QString(tr("PLUG: %1")).arg(name));

    current_name=name;

    amp->load(amplifier_set);
    if(settings.value("Settings/popupChangedWindows").toBool())
        amp->show();
    for(int i = 0; i < 4; i++)
    {
        switch(effects_set[i].fx_slot)
        {
        case 0x00:
        case 0x04:
            effect1->load(effects_set[i]);
            if(effects_set[i].effect_num)
                if(settings.value("Settings/popupChangedWindows").toBool())
                    effect1->show();
            break;

        case 0x01:
        case 0x05:
            effect2->load(effects_set[i]);
            if(effects_set[i].effect_num)
                if(settings.value("Settings/popupChangedWindows").toBool())
                    effect2->show();
            break;

        case 0x02:
        case 0x06:
            effect3->load(effects_set[i]);
            if(effects_set[i].effect_num)
                if(settings.value("Settings/popupChangedWindows").toBool())
                    effect3->show();
            break;

        case 0x03:
        case 0x07:
            effect4->load(effects_set[i]);
            if(effects_set[i].effect_num)
                if(settings.value("Settings/popupChangedWindows").toBool())
                    effect4->show();
            break;
        }
    }

    return 0;
}

// activate buttons
void MainWindow::enable_buttons(void)
{
    amp->enable_set_button(true);
    effect1->enable_set_button(true);
    effect2->enable_set_button(true);
    effect3->enable_set_button(true);
    effect4->enable_set_button(true);
    ui->actionConnect->setDisabled(false);
    ui->actionDisconnect->setDisabled(false);
    ui->actionSave_to_amplifier->setDisabled(false);
    ui->action_Load_from_amplifier->setDisabled(false);
    ui->actionSave_effects->setDisabled(false);
    ui->action_Library_view->setDisabled(false);
}

void MainWindow::check_for_updates()
{
    if(sender() == ui->actionCheck_for_Updates)
        manual_check = true;
    else
        manual_check = false;

    QNetworkAccessManager *qnam = new QNetworkAccessManager(this);
    reply = qnam->get(QNetworkRequest((QUrl)"http://piorekf.org/plug/VERSION"));
    connect(reply, SIGNAL(finished()), this, SLOT(httpReadyRead()));
}

void MainWindow::httpReadyRead()
{
    if(reply->readAll() > VERSION)
    {
        QLabel *label = new QLabel(tr("<b>Update available!</b>"), this);
        ui->statusBar->addWidget(label);
        QMessageBox::information(this, "Update", "<b>Update available!</b><br><br>Check homepage for new version.");
    }
    else if(manual_check)
        ui->statusBar->showMessage(tr("You are using the newest version"), 5000);
}

void MainWindow::change_name(int slot, QString *name)
{
    load->change_name(slot, name);
    quickpres->change_name(slot, name);
}

void MainWindow::set_index(int value)
{
    save->change_index(value, current_name);
}

void MainWindow::save_effects(int slot, char *name, int fx_num, bool mod, bool dly, bool rev)
{
    struct fx_pedal_settings effects[2];

    if(fx_num == 1)
    {
        if(mod)
        {
            effect2->get_settings(effects[0]);
            set_effect(effects[0]);
        }
        else if(dly)
        {
            effect3->get_settings(effects[0]);
            set_effect(effects[0]);
        }
        else if(rev)
        {
            effect4->get_settings(effects[0]);
            set_effect(effects[0]);
        }
    }
    else
    {
        effect3->get_settings(effects[0]);
        set_effect(effects[0]);
        effect4->get_settings(effects[1]);
        set_effect(effects[1]);
    }

    amp_ops->save_effects(slot, name, fx_num, effects);
}

void MainWindow::loadfile(QString filename)
{
    if(filename.isEmpty())
        filename = QFileDialog::getOpenFileName(this, tr("Open..."), QDir::homePath(), tr("FUSE files (*.fuse *.xml)"));

    if(filename.isEmpty())
        return;

    QFile *file = new QFile(filename, this);

    if(file->exists())
    {
        if (!file->open(QFile::ReadOnly | QFile::Text))
        {
            QMessageBox::critical(this, tr("Error!"), tr("Could not open file"));
            return;
        }
    }
    else
    {
        QMessageBox::critical(this, tr("Error!"), tr("No such file"));
        return;
    }

    QSettings settings;
    struct amp_settings amplifier_set;
    struct fx_pedal_settings effects_set[4];
    QString name;
    LoadFromFile *loader = new LoadFromFile(file, &name, &amplifier_set, effects_set);

    loader->loadfile();
    file->close();
    delete loader;
    delete file;

    change_title(name);

    amp->load(amplifier_set);
    if(connected)
        amp->send_amp();
    if(settings.value("Settings/popupChangedWindows").toBool())
        amp->show();
    for(int i = 0; i < 4; i++)
    {
        switch(effects_set[i].fx_slot)
        {
        case 0x00:
        case 0x04:
            effect1->load(effects_set[i]);
            if(connected)
                effect1->send_fx();
            if(effects_set[i].effect_num)
                if(settings.value("Settings/popupChangedWindows").toBool())
                    effect1->show();
            break;

        case 0x01:
        case 0x05:
            effect2->load(effects_set[i]);
            if(connected)
                effect2->send_fx();
            if(effects_set[i].effect_num)
                if(settings.value("Settings/popupChangedWindows").toBool())
                    effect2->show();
            break;

        case 0x02:
        case 0x06:
            effect3->load(effects_set[i]);
            if(connected)
                effect3->send_fx();
            if(effects_set[i].effect_num)
                if(settings.value("Settings/popupChangedWindows").toBool())
                    effect3->show();
            break;

        case 0x03:
        case 0x07:
            effect4->load(effects_set[i]);
            if(connected)
                effect4->send_fx();
            if(effects_set[i].effect_num)
                if(settings.value("Settings/popupChangedWindows").toBool())
                    effect4->show();
            break;
        }
    }
}

void MainWindow::get_settings(struct amp_settings *amplifier_settings, struct fx_pedal_settings fx_settings[4])
{
    if(amplifier_settings != NULL)
        amp->get_settings(amplifier_settings);
    if(fx_settings != NULL)
    {
        effect1->get_settings(fx_settings[0]);
        effect2->get_settings(fx_settings[1]);
        effect3->get_settings(fx_settings[2]);
        effect4->get_settings(fx_settings[3]);
    }
}

void MainWindow::change_title(QString name)
{
    current_name = name;

    if(current_name.isEmpty())
        setWindowTitle(QString(tr("PLUG: NONE")));
    else
        setWindowTitle(QString(tr("PLUG: %1")).arg(current_name));
}

void MainWindow::show_fx1()
{
    if(!effect1->isVisible())
        effect1->show();
    effect1->activateWindow();
}

void MainWindow::show_fx2()
{
    if(!effect2->isVisible())
        effect2->show();
    effect2->activateWindow();
}
void MainWindow::show_fx3()
{
    if(!effect3->isVisible())
        effect3->show();
    effect3->activateWindow();
}
void MainWindow::show_fx4()
{
    if(!effect4->isVisible())
        effect4->show();
    effect4->activateWindow();
}

void MainWindow::show_amp()
{
    if(!amp->isVisible())
        amp->show();
    amp->activateWindow();
}

void MainWindow::show_library()
{
    QSettings settings;
    bool previous = settings.value("Settings/popupChangedWindows").toBool();

    settings.setValue("Settings/popupChangedWindows", false);

    library = new Library(names, this);
    effect1->close();
    effect2->close();
    effect3->close();
    effect4->close();
    amp->close();
    this->close();
    library->exec();

    settings.setValue("Settings/popupChangedWindows", previous);
    this->show();
    delete library;
}

void MainWindow::update_firmware()
{
//    if(QMessageBox::warning(this, "Danger!", "This function may destroy your amplifier!<br><u><b>You are using it at you own risk!</b></u>", QMessageBox::Ok | QMessageBox::Cancel, QMessageBox::Cancel) == QMessageBox::Cancel)
//        return;

    QString filename;
    int ret = 0;

    QMessageBox::information(this, "Prepare", "Please power off the amplifier, then power it back on while holding \"Save\" button.<br>After pressing \"OK\" choose firmware file and then update will begin.<br>It will take about one minute. You will be notified when it's finished.");

    filename = QFileDialog::getOpenFileName(this, tr("Open..."), QDir::homePath(), tr("Mustang firmware (*.upd)"));
    if(filename.isEmpty())
        return;

    if(connected)
        this->stop_amp();

    ui->statusBar->showMessage("Updating firmware. Please wait...");
    ui->centralWidget->setDisabled(true);
    ui->menuBar->setDisabled(true);
    this->repaint();
    ret = amp_ops->update(filename.toAscii().data());  // magic part
    ui->centralWidget->setDisabled(false);
    ui->menuBar->setDisabled(false);
    ui->statusBar->showMessage("", 1);
    if(ret == -100)
    {
        ui->statusBar->showMessage(tr("Error: Suitable device not found!"), 5000);
        return;
    }
    else if(ret)
    {
        ui->statusBar->showMessage(QString(tr("Communication error: %1")).arg(ret), 5000);
        return;
    }
    QMessageBox::information(this, "Update finished", "<b>Update finished</b><br>If \"Exit\" button is lit - update was succesful<br>If \"Save\" button is lit - update failed<br><br>Power off the amplifier and then back on to finish the process.");
}

void MainWindow::show_default_effects()
{
    deffx = new DefaultEffects(this);
    deffx->exec();
    delete deffx;
}

void MainWindow::empty_other(int value, Effect *caller)
{
    struct fx_pedal_settings settings;
    int fx_family = check_fx_family(value);

    if(caller != effect1)
    {
        effect1->get_settings(settings);
        if(check_fx_family(settings.effect_num) == fx_family)
        {
            effect1->choose_fx(0);
            effect1->send_fx();
        }
    }

    if(caller != effect2)
    {
        effect2->get_settings(settings);
        if(check_fx_family(settings.effect_num) == fx_family)
        {
            effect2->choose_fx(0);
            effect2->send_fx();
        }
    }

    if(caller != effect3)
    {
        effect3->get_settings(settings);
        if(check_fx_family(settings.effect_num) == fx_family)
        {
            effect3->choose_fx(0);
            effect3->send_fx();
        }
    }

    if(caller != effect4)
    {
        effect4->get_settings(settings);
        if(check_fx_family(settings.effect_num) == fx_family)
        {
            effect4->choose_fx(0);
            effect4->send_fx();
        }
    }
}

int MainWindow::check_fx_family(int value)
{
    if(value == 0)
        return 0;
    if(value >= 1 && value <= 7)
        return 1;
    if(value >= 8 && value <= 18)
        return 2;
    if(value >= 19 && value <= 27)
        return 3;
    if(value >= 28 && value <= 37)
        return 4;

    return 0;
}

void MainWindow::load_presets0()
{
  QSettings settings;

  if(settings.contains("DefaultPresets/Preset0"))
      load_from_amp(settings.value("DefaultPresets/Preset0").toInt());
}

void MainWindow::load_presets1()
{
  QSettings settings;

  if(settings.contains("DefaultPresets/Preset1"))
      load_from_amp(settings.value("DefaultPresets/Preset1").toInt());
}

void MainWindow::load_presets2()
{
  QSettings settings;

  if(settings.contains("DefaultPresets/Preset2"))
      load_from_amp(settings.value("DefaultPresets/Preset2").toInt());
}

void MainWindow::load_presets3()
{
  QSettings settings;

  if(settings.contains("DefaultPresets/Preset3"))
      load_from_amp(settings.value("DefaultPresets/Preset3").toInt());
}

void MainWindow::load_presets4()
{
  QSettings settings;

  if(settings.contains("DefaultPresets/Preset4"))
      load_from_amp(settings.value("DefaultPresets/Preset4").toInt());
}

void MainWindow::load_presets5()
{
  QSettings settings;

  if(settings.contains("DefaultPresets/Preset5"))
      load_from_amp(settings.value("DefaultPresets/Preset5").toInt());
}

void MainWindow::load_presets6()
{
  QSettings settings;

  if(settings.contains("DefaultPresets/Preset6"))
      load_from_amp(settings.value("DefaultPresets/Preset6").toInt());
}

void MainWindow::load_presets7()
{
  QSettings settings;

  if(settings.contains("DefaultPresets/Preset7"))
      load_from_amp(settings.value("DefaultPresets/Preset7").toInt());
}

void MainWindow::load_presets8()
{
  QSettings settings;

  if(settings.contains("DefaultPresets/Preset8"))
      load_from_amp(settings.value("DefaultPresets/Preset8").toInt());
}

void MainWindow::load_presets9()
{
  QSettings settings;

  if(settings.contains("DefaultPresets/Preset9"))
      load_from_amp(settings.value("DefaultPresets/Preset9").toInt());
}
