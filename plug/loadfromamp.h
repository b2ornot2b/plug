#ifndef LOADFROMAMP_H
#define LOADFROMAMP_H

#include <QMainWindow>

namespace Ui {
    class LoadFromAmp;
}

class LoadFromAmp : public QMainWindow
{
    Q_OBJECT

public:
    explicit LoadFromAmp(QWidget *parent = 0);
    ~LoadFromAmp();
    void load_names(char names[24][32]);
    void change_name(int, QString *);

private:
    Ui::LoadFromAmp *ui;

private slots:
    void load(void);
};

#endif // LOADFROMAMP_H
