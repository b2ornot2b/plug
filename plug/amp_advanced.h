#ifndef AMP_ADVANCED_H
#define AMP_ADVANCED_H

#include <QDialog>

namespace Ui {
    class Amp_Advanced;
}

class Amp_Advanced : public QDialog
{
    Q_OBJECT

public:
    explicit Amp_Advanced(QWidget *parent = 0);
    ~Amp_Advanced();

private:
    Ui::Amp_Advanced *ui;

public slots:
    void change_cabinet(int);
    void change_noise_gate(int);

private slots:
    void activate_custom_ng(int);
};

#endif // AMP_ADVANCED_H