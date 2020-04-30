#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QFile>
#include <QProcess>
#include <QMessageBox>
#include <QDebug>
#include <QVector>
#include <QTextStream>
#include <QStringList>
#include <QThread>
#include <QDir>
#include <QInputDialog>
#include <QListWidgetItem>
#include <QCollator>

//#include <iostream>
//#include <algorithm>

using namespace std;


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    //start mediaclient in case it did not...
    QProcess::execute("/opt/bin/mediaclient --start");
    QThread::msleep(3000);

    ui->setupUi(this);

    // start DAB default ##################################################################################################################

    tgl_state = "DAB";

    //qDebug() << "state: " << tgl_state;
    //qDebug() << "button: " << ui->tgl_dab_fm->text();
    QProcess::execute("/opt/bin/mediaclient -d /dev/dab0");
    QProcess::execute("/opt/bin/mediaclient -m DAB -g off");
    ui->ls_dab->setVisible(true);
    ui->ls_fm->setVisible(false);
    ui->btn_rename->setVisible(false);
    ui->ln_man_tune->setVisible(false);
    ui->label->setVisible(false);
    ui->btn_add->setVisible(false);
    ui->btn_man_tune->setVisible(false);

    //disable tunebutton on start
    ui->btn_tune->setDisabled(true);

    MainWindow::fill_list();
    MainWindow::dab_list();

    //MainWindow::fill_list();
    MainWindow::fm_list();
}


MainWindow::~MainWindow()
{
    //kill mediaclient on close, else sound goes on...
    QProcess::execute("/opt/bin/mediaclient -m DAB -g on");
    QProcess::execute("/opt/bin/mediaclient -m RADIO -g on");
    QProcess::execute("/opt/bin/mediaclient --shutdown");

    delete ui;
}

void MainWindow::on_tgl_dab_fm_clicked()
{
    if(ui->tgl_dab_fm->text() == "FM"){
        ui->tgl_dab_fm->setText("DAB");
        tgl_state = "FM";
        ui->ls_dab->setVisible(false);
        ui->ls_fm->setVisible(true);
        ui->btn_rename->setVisible(true);
        ui->ln_man_tune->setVisible(true);
        ui->label->setVisible(true);
        ui->btn_add->setVisible(true);
        ui->btn_man_tune->setVisible(true);
    } else {
        ui->tgl_dab_fm->setText("FM");
        tgl_state = "DAB";
        ui->ls_dab->setVisible(true);
        ui->ls_fm->setVisible(false);
        ui->btn_rename->setVisible(false);
        ui->ln_man_tune->setVisible(false);
        ui->label->setVisible(false);
        ui->btn_add->setVisible(false);
        ui->btn_man_tune->setVisible(false);
    }

    //enable/disable tune button depending on station selected or not
    int init_fm_list = ui->ls_fm->currentRow();
    int init_dab_list = ui->ls_dab->currentRow();

    if(init_fm_list == -1 && tgl_state == "FM"){
        ui->btn_tune->setDisabled(true);
    }

    if(init_fm_list != -1 && tgl_state == "FM"){
        ui->btn_tune->setDisabled(false);
    }

    if(init_dab_list == -1 && tgl_state == "DAB"){
        ui->btn_tune->setDisabled(true);
    }

    if(init_dab_list != -1 && tgl_state == "DAB"){
        ui->btn_tune->setDisabled(false);
    }
}

void MainWindow::on_btn_scan_clicked()
{
    //DAB scan

    if (tgl_state == "DAB"){

        //clear list
        ui->ls_dab->clear();
        //clear vectors
        dab.clear();

        //starte prozess scan dab frequenzen
        QProcess process;
        process.close();
        process.start("/opt/bin/mediaclient --scandabfrequencies /dev/dab0");
        process.waitForFinished();
        //process.close();
        //QByteArray scanned_dab(process.readAllStandardOutput());
        QString scanned_dab(process.readAllStandardOutput());
        process.close();

        // output(process.readAllStandardOutput());
        //QByteArray lock;
        QList<QString> locked_dab; //list with linenumbers found above [LOCKED]
        QList<QString> locked_freq_dab; //list with frequencies which were found above [LOCKED]
        QList<QString> freq_avail; //list with cleaned frequencies
        int k = 0;
        //
        QTextStream stream_dab(&scanned_dab);
        QTextStream console_count_lines_freq_dab(&scanned_dab);
        QString line_dab;

        //QString line = stream_dab.readLine();
        QString line_lock = stream_dab.readLine();
        //int line_count = 0;
        int line_count_dab = 0;
/*
        while (!stream_dab.atEnd()) {
            line_dab = stream_dab.readLine();
            //ui->listScanOrg->addItem(line);
            line_count++;
            qDebug() << "line_count" << line_count;
        }
*/
        //zähle lines der gefundenen frequenzen

        //austausch
//        QString linea[100];
//        QTextStream in(&scanned_dab);

//        while (!in.atEnd()){

//            linea[line_count] = in.readLine();
//            line_count++;

//        };

        while (!console_count_lines_freq_dab.atEnd()) {
            QString line_dab = console_count_lines_freq_dab.readLine();
            line_count_dab++;
        }


        //QString lineb = QString::number(line_count);

        //finde lines die LOCKED sind
        for (int i = 0; i < line_count_dab; i++){

            // austausch
              line_lock = stream_dab.readLine();
              locked_freq_dab.append(line_lock);

              if (line_lock == "[LOCKED]"){

                  k = i-1;
                  locked_dab.append(QString::number(k));
                  //qDebug() <<"position locked" << locked_dab;

                  //lock.append(k);

                //ui->listScan->addItem(QString::number(i));
                  //ui->listScan->addItem(lock);
                 // ui->lineEdit_2->setText(lock.indexOf(i));
               }



            //line_lock = stream_dab.readLine();




        }
        qDebug() <<"position locked" << locked_dab;

        for (int i = 0; i < locked_dab.size(); i++){
            //int tmp = (locked_freq_dab.at(i).toInt());
            QString holder = locked_dab.at(i);
            //int hold = holder.toInt;
            QString fr = locked_freq_dab.at(holder.toInt());
            QStringRef fr_part = fr.leftRef(4);

            if(fr_part.contains(" ")){
                fr = fr.remove(0, 3);
            } else {
                //untested, no freq here with more then 3 characters in front
                fr = fr.remove(0, 4);
            }

            freq_avail.append(fr);


            //ui->listScan->addItem(fr);
        }
        qDebug() << "freq avail:" << freq_avail;

        //tune zu transponder

        //open file and append found transponders

        QFile file_dab(path_dab);
        if(!file_dab.open(QFile::WriteOnly | QFile::Text)){
            //QMessageBox::warning(this,"..","keine datei gefunden");
            //ui->warn_no_dab_list->setVisible(true);
            return;
        }

        //clear file before writing
        //file_dab.resize(0);
        QTextStream out(&file_dab);

        //QString transponder = trans.at(0);
        QStringList unsort_scan_dab;

        for(int i = 0; i < freq_avail.size(); i++){

//                QProcess::execute("/opt/bin/mediaclient -m DAB -f " + freq_avail.at(i));
//                QThread::msleep(500);

            //process tune to DAB frequency
            QProcess process_dab_trans_1;
            process_dab_trans_1.close();
            process_dab_trans_1.start("/opt/bin/mediaclient -m DAB -f " + freq_avail.at(i));
            process_dab_trans_1.waitForFinished();
            QThread::msleep(3000); //important, else old frequency is scanned again
            process_dab_trans_1.close();

            //process scan transponders at frequency
            QProcess process_dab_trans_2;
            process_dab_trans_2.close();
            process_dab_trans_2.start("/opt/bin/mediaclient -m DAB -f " + freq_avail.at(i) + " --scandabservices /dev/dab0");
            process_dab_trans_2.waitForFinished();

            QString output_dab_trans(process_dab_trans_2.readAllStandardOutput());
            process_dab_trans_2.close();

            QTextStream stream_dab_trans(&output_dab_trans);
            //QStringList unsort_scan_dab;

            //QString line_dab_trans;

                while (!stream_dab_trans.atEnd()) {
                      QString line_dab_trans = stream_dab_trans.readLine();

                      if(!line_dab_trans.contains("Service Name")){

                          //"0 x char char char char"
                          QRegularExpression sid_re("[0][x][a-f0-9][a-f0-9][a-f0-9][a-f0-9]");
                          QRegularExpressionMatch match = sid_re.match(line_dab_trans);
                          QString matched;
                          if (match.hasMatch()) {
                              matched = match.captured(0);
                          }

                          QString service_name = line_dab_trans.left(line_dab_trans.indexOf(QLatin1String("0x")));
                          //QStringList unsort_scan_dab;

                          /*
                          QStringList one_line = line_dab_trans.split('\t');
                          QString one = one_line;
                          ui->listScan->addItem(one_line);
                          */

                          //QString line = in.readLine();
                          //if(!line.contains(delete_marked, Qt::CaseSensitive)){
                          //QString outline = line;

                          //if(service_name != "Service Name"){
                              ////out << freq_avail.at(i) << "," << service_name << "," << matched << "\n";
                          QString tmp = service_name + "," + freq_avail.at(i) + "," + matched;
                          unsort_scan_dab.append(tmp);
                              //qDebug() << freq_avail.at(i) << service_name << matched;
                          //}

                           //clear line_dab_trans at the end, else only transponders from frequency #0 aer listed
                          line_dab_trans.clear();
                      }
                }

        }
        QStringList list {MainWindow::sort_list(unsort_scan_dab)};

        foreach(QString dab_line_sort, list){
            out << dab_line_sort << "\n";
        }

        file_dab.flush();
        file_dab.close();


        //nochmal prüfen ob notwendig...
        if(!file_dab.open(QFile::ReadOnly | QFile::Text)){
            //QMessageBox::warning(this,"no stationfile found","Please hit scan first!");
            //ui->warn_no_dab_list->setVisible(true);
            return;
        }

        QTextStream in_file(&file_dab);
        QString text;

        while (!in_file.atEnd()) {
                   text = in_file.readLine();
                   //qDebug() << text;

                   QStringList split_text = text.split(",");
                   //qDebug() << split_text;

                       QVector<QString> dab_row;

                       dab_row.push_back(split_text.at(0));
                       dab_row.push_back(split_text.at(1));
                       dab_row.push_back(split_text.at(2));
                       qDebug() << "dab row scan" << dab_row;

                       dab.push_back(dab_row);
                       qDebug() << "dab scan" << dab;

         }

        file_dab.close();

        MainWindow::dab_list();

    }
    //FM scan #############################################################################################################################

    if (tgl_state == "FM"){
        //clear list
        ui->ls_fm->clear();
        //clear vectors
        fm.clear();
        qDebug() << "content vector fm:" << fm;

        //start scanning fm ###############################################################################################################
        QProcess process_fm;
        process_fm.close();
        process_fm.start("/opt/bin/mediaclient --scanfmfrequencies /dev/radio0");
        process_fm.waitForFinished();
        //process.close();
        //QByteArray scanned_dab(process.readAllStandardOutput());
        QString scanned_fm(process_fm.readAllStandardOutput());
        process_fm.close();

        QTextStream console_count_lines_fm(&scanned_fm);
        QTextStream console_find_freq(&scanned_fm);
        //QString line_fm;

        QString line_lock = console_count_lines_fm.readLine();
        int line_count_fm = 0;

        while (!console_count_lines_fm.atEnd()) {
            QString line_fm = console_count_lines_fm.readLine();
            line_count_fm++;
        }

        QFile file_fm(path_fm);
        if(!file_fm.open(QFile::WriteOnly | QFile::Text)){
            //QMessageBox::warning(this,"..","keine datei gefunden");
            //ui->warn_no_dab_list->setVisible(true);
            return;
        }
        QTextStream out(&file_fm);
        QStringList unsort_scan_fm;

        for(int i = 0; i < line_count_fm; i++){
            QString tmp = console_find_freq.readLine();
            //if(tmp != "" && tmp != "Scan completed" && tmp != "SCAN SETUP"){
            //QString match = "LOCKED";
            if(tmp.contains("LOCKED")){
                tmp.replace(" [LOCKED]", "000");
                float mhz = tmp.toFloat() / 1000000;
                //QString mhz_str = QString::number(mhz).right();
                tmp = "Station " + QString::number(i-1) + " " + QString::number(mhz) + " MHz," + tmp;

                //out << tmp << "\n";

                unsort_scan_fm.append(tmp);
                //qDebug() << "unsort list:" << unsort_scan_fm;
            }
        }

        QStringList list {MainWindow::sort_list(unsort_scan_fm)};

        foreach(QString fm_line_sort, list){
            out << fm_line_sort << "\n";
        }

        file_fm.flush();
        file_fm.close();

        //read frequencies QVector ####################################################################################################
        //QFile file_fm(path_fm);
        if(!file_fm.open(QFile::ReadOnly | QFile::Text)){
            //QMessageBox::warning(this,"..","keine datei gefunden");
            //ui->warn_no_dab_list->setVisible(true);
            return;
        }

        QTextStream in_file_fm(&file_fm);

        while (!in_file_fm.atEnd()) {
            QString text_fm = in_file_fm.readLine();

            QStringList split_text_fm = text_fm.split(",");
            //qDebug() << split_text_fm;

            QVector<QString> fm_row;

            fm_row.push_back(split_text_fm.at(0));
            fm_row.push_back(split_text_fm.at(1));

            fm.push_back(fm_row);
            //qDebug() << "fm scan" << fm;

        }
        file_fm.close();

        MainWindow::fm_list();
    }
}

//#########################################################################################################################################
//Qt Widgets and buttons

void MainWindow::on_btn_tune_clicked()
{
    MainWindow::tune();
}

void MainWindow::on_btn_kill_clicked()
{
    QProcess::execute("/opt/bin/mediaclient --shutdown");
}

void MainWindow::on_btn_restart_clicked()
{
    QProcess::execute("/opt/bin/mediaclient --start");
}

void MainWindow::on_ls_fm_itemSelectionChanged()
{
    ui->btn_tune->setEnabled(true);
}

void MainWindow::on_ls_dab_itemSelectionChanged()
{
    ui->btn_tune->setEnabled(true);
}

void MainWindow::on_btn_delete_clicked()
{
    MainWindow::delete_line();
}

void MainWindow::on_btn_rename_clicked()
{
    MainWindow::rename();
}

void MainWindow::on_btn_add_clicked()
{
    MainWindow::add_station();
}

void MainWindow::on_btn_man_tune_clicked()
{

}

//#########################################################################################################################################
//functions

//void MainWindow::testfunction(){

//    QMessageBox::warning(this,"no stationlist for DAB found","no stationlist for DAB found\nPlease hit scan in mode DAB first!");
//}

void MainWindow::fm_list(){
    for(int i = 0; i < fm.size(); i++){
      ui->ls_fm->addItem(fm[i][0]);
      //ui->ls_fm->sortItems(Qt::AscendingOrder);
    }
}

void MainWindow::dab_list(){
    for(int i = 0; i < dab.size(); i++){
      ui->ls_dab->addItem(dab[i][0]);
      //ui->ls_dab->sortItems(Qt::AscendingOrder);
    }
}

void MainWindow::tune(){

    QString radio_dab_type;

    //DAB #################################################################################################################################
    if (tgl_state == "DAB"){

        radio_dab_type = "DAB";
        int ind_marked = ui->ls_dab->currentRow();
//        if(ind_marked > -1){

            freq = dab[ind_marked][1];
            serv_id = dab[ind_marked][2];
//        }
    }

    //FM ##################################################################################################################################
    if (tgl_state == "FM"){

        radio_dab_type = "RADIO";
        int ind_marked = ui->ls_fm->currentRow();
        //if(ind_marked > -1){

        freq = fm[ind_marked][1];
        //}
    }

    //start mediaclient in case it did not...
    QProcess::execute("/opt/bin/mediaclient --start");
    QProcess::execute("/opt/bin/mediaclient -m" + radio_dab_type + " -f" + freq);
    if(tgl_state == "DAB"){
        QProcess::execute("/opt/bin/mediaclient -m " + radio_dab_type + " -f " + freq + " --sid " + serv_id);
    }
    QProcess::execute("/opt/bin/mediaclient -m " + radio_dab_type + " -g off");
}

void MainWindow::delete_line(){

    //DAB #################################################################################################################################
    if (tgl_state == "DAB"){

        QString delete_marked = ui->ls_dab->currentItem()->text();

        QFile out_tmp("../tmp.txt");
        QFile file_dab(path_dab);

        if(!file_dab.open(QFile::ReadOnly | QFile::Text)){
            //QMessageBox::warning(this,"..","keine datei gefunden");
            return;
        }

         if(!out_tmp.open(QFile::WriteOnly | QFile::Text)){
             //QMessageBox::warning(this,"..","keine datei gefunden");
             return;
         }

        QTextStream in_file_dab(&file_dab);
        QTextStream out(&out_tmp);

        while(!in_file_dab.atEnd()){
            QString line = in_file_dab.readLine();
            if(!line.contains(delete_marked, Qt::CaseSensitive)){
            QString outline = line;
                    out << outline << "\n";
            }
        }

        file_dab.close();
        out_tmp.flush();
        out_tmp.close();

        file_dab.remove();
        out_tmp.rename(path_dab);

        ui->ls_dab->clear();

        //mute DAB stream, else deleted entry is still active
        //todo, mute only if station to delete is played station
        QProcess::execute("/opt/bin/mediaclient -m DAB -g on");

        MainWindow::fill_list();
        MainWindow::dab_list();
    }

    //FM ##################################################################################################################################
    if (tgl_state == "FM"){

        QString delete_marked = ui->ls_fm->currentItem()->text();

        QFile out_tmp("../tmp.txt");
        QFile file_fm(path_fm);
        if(!file_fm.open(QFile::ReadOnly | QFile::Text)){
            //QMessageBox::warning(this,"..","keine datei gefunden");
            return;
        }

         if(!out_tmp.open(QFile::WriteOnly | QFile::Text)){
             //QMessageBox::warning(this,"..","keine datei gefunden");
             return;
         }

        QTextStream in_file_fm(&file_fm);
        QTextStream out(&out_tmp);

        while(!in_file_fm.atEnd()){
            QString line = in_file_fm.readLine();
            if(!line.contains(delete_marked, Qt::CaseSensitive)){
            QString outline = line;
                    out << outline << "\n";
            }
        }

        file_fm.close();
        out_tmp.flush();
        out_tmp.close();

        file_fm.remove();
        out_tmp.rename(path_fm);

        ui->ls_fm->clear();

        //mute RADIO stream, else deleted entry is still active
        //todo, mute only if station to delete is played station
        QProcess::execute("/opt/bin/mediaclient -m RADIO -g on");

        MainWindow::fill_list();
        MainWindow::fm_list();
    }
}

void MainWindow::fill_list(){

    //DAB #################################################################################################################################

    QFile file_dab(path_dab);

    if(!file_dab.open(QFile::ReadOnly | QFile::Text)){
        QMessageBox::warning(this,"no stationlist for DAB found","no stationlist for DAB found\nPlease hit scan in mode DAB first!");
        //ui->warn_no_dab_list->setVisible(true);
        return;
    }

    dab.clear();

    QTextStream in_file_dab(&file_dab);
    QString text_dab;

    while (!in_file_dab.atEnd()) {

       text_dab = in_file_dab.readLine();

       QStringList split_text_dab = text_dab.split(",");

       QVector<QString> dab_row;

       dab_row.push_back(split_text_dab.at(0));
       dab_row.push_back(split_text_dab.at(1));
       dab_row.push_back(split_text_dab.at(2));
       dab.push_back(dab_row);
    }

    file_dab.close();

    //FM ##################################################################################################################################

    QFile file_fm(path_fm);

    if(!file_fm.open(QFile::ReadOnly | QFile::Text)){
        QMessageBox::warning(this,"no stationlist for FM found","no stationlist for FM found\nPlease hit scan in mode FM first!");
        //ui->warn_no_dab_list->setVisible(true);
        return;
    }

    fm.clear();

    QTextStream in_file_fm(&file_fm);
    QString text_fm;

    while (!in_file_fm.atEnd()) {

       text_fm = in_file_fm.readLine();

       QStringList split_text_fm = text_fm.split(",");

       QVector<QString> fm_row;

       fm_row.push_back(split_text_fm.at(0));
       fm_row.push_back(split_text_fm.at(1));
       fm.push_back(fm_row);
    }

    file_fm.close();
}

void MainWindow::rename(){

    //FM ##################################################################################################################################
    if (tgl_state == "FM"){

        ui->btn_rename->setVisible(true);

        //int ind_marked = ui->ls_fm->currentRow();
        //if(ind_marked > -1){

        QString rename_marked = ui->ls_fm->currentItem()->text();
        bool ok;

        QString new_name = QInputDialog::getText(this, "enter new name", "new name: ",QLineEdit::Normal,rename_marked, &ok);

        if(ok && !new_name.isEmpty()){
            QFile out_tmp("../tmp.txt");
            QFile file_fm(path_fm);
            if(!file_fm.open(QFile::ReadOnly | QFile::Text)){
                //QMessageBox::warning(this,"..","keine datei gefunden");
                return;
            }

            if(!out_tmp.open(QFile::WriteOnly | QFile::Text)){
                 //QMessageBox::warning(this,"..","keine datei gefunden");
                return;
            }

            QTextStream in_file_fm(&file_fm);
            QTextStream out(&out_tmp);
            QStringList unsort_rename_fm;

            while(!in_file_fm.atEnd()){
                QString line = in_file_fm.readLine();
                QStringList line_split = line.split(",");
                if(line_split.at(0) == rename_marked){

                QString outline = new_name + "," + line_split.at(1);
                    unsort_rename_fm.append(outline);
                } else {
                QString outline = line;
                    unsort_rename_fm.append(outline);
                }
            }

            QStringList list {MainWindow::sort_list(unsort_rename_fm)};

            foreach(QString fm_line_sort, list){
                out << fm_line_sort << "\n";
            }

            file_fm.close();
            out_tmp.flush();
            out_tmp.close();

            file_fm.remove();
            out_tmp.rename(path_fm);

            ui->ls_fm->clear();

            //mute RADIO stream, else deleted entry is still active
            //QProcess::execute("/opt/bin/mediaclient -m RADIO -g on");

            MainWindow::fill_list();
            MainWindow::fm_list();
        }
    }
}

void MainWindow::add_station(){

    //FM ##################################################################################################################################
    if (tgl_state == "FM"){

        ui->btn_rename->setVisible(true);

        QString add_station = ui->ln_man_tune->text();
        float add_station_float = add_station.toFloat();
        int multi = add_station_float * 1000000;
        QString station_conv_string = (QString::number(multi));

        if(!add_station.isEmpty()){
            QFile out_tmp("../tmp.txt");
            QFile file_fm(path_fm);

            if(!file_fm.open(QFile::ReadOnly | QFile::Text)){
                //QMessageBox::warning(this,"..","keine datei gefunden");
                return;
            }

            if(!out_tmp.open(QFile::WriteOnly | QFile::Text)){
                 //QMessageBox::warning(this,"..","keine datei gefunden");
                return;
            }

            QTextStream in_file_fm(&file_fm);
            file_fm.resize(0);
            QTextStream out(&out_tmp);

            QStringList unsort_add_fm;

            QCollator coll;
            coll.setNumericMode(true);

            while(!in_file_fm.atEnd()){
                QString line = in_file_fm.readLine();
                //if(!line.contains(delete_marked, Qt::CaseSensitive)){
                //QStringList line_split = line.split(",");

                QString outline = line;
                        //out << outline << "\n";

                unsort_add_fm.append(outline);
            }

            if(add_station.contains(",")){
                add_station = add_station.replace(",", ".");
            }
            if(!add_station.contains(".")){
                add_station = add_station.append(".0");
            }

            //add new station at end of file
            QString outline = "man station@ " + add_station + "MHz," + station_conv_string;
                    //out << outline << "\n";

            unsort_add_fm.append(outline);

            QStringList list {MainWindow::sort_list(unsort_add_fm)};

            foreach(QString fm_line_sort, list){
                out << fm_line_sort << "\n";
            }

            file_fm.close();
            out_tmp.flush();
            out_tmp.close();

            file_fm.remove();            
            out_tmp.rename(path_fm);

            ui->ls_fm->clear();

            MainWindow::fill_list();
            MainWindow::fm_list();
        }
    }
}

QStringList MainWindow::sort_list(QStringList list){

    QCollator coll;
    coll.setNumericMode(true);

    std::sort(list.begin(), list.end(), [&](const QString& s1, const QString& s2){ return coll.compare(s1, s2) < 0; });

    return list;
}

