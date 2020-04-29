#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QListWidgetItem>
//#include <iostream>
//#include <algorithm>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_tgl_dab_fm_clicked();

    void on_btn_scan_clicked();

    void on_btn_tune_clicked();

    void on_btn_kill_clicked();

    void on_btn_restart_clicked();

    void on_ls_fm_itemSelectionChanged();

    void on_ls_dab_itemSelectionChanged();

    void on_btn_rename_clicked();

    void on_btn_delete_clicked();

    void on_btn_add_clicked();

    void on_btn_man_tune_clicked();

//    void on_btn_clear_clicked();

//    void testfunction();

    void fm_list();

    void dab_list();

    void tune();

    void delete_line();

    void fill_list();

    void rename();

    void add_station();

    QStringList sort_list(QStringList list);

    //void on_ls_dab_itemPressed(QListWidgetItem *item);

    //void on_ls_fm_itemPressed(QListWidgetItem *item);

private:
    Ui::MainWindow *ui;
    QString path_dab = "../.dab.txt";
    QString path_fm = "../.fm.txt";

    QString tgl_dab_fm = "FM";
    QString tgl_state;

    QVector<QVector<QString>> dab;
    //QVector<QString> dab_row;

    QVector<QVector<QString>> fm;
    //QVector<QString> fm;
    //QVector<QString> fm_row;

    QString freq;
    QString serv_id;

};

#endif // MAINWINDOW_H
