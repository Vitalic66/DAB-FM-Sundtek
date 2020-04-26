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


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{

    //start mediaclient in case it did not...
    QProcess::execute("/opt/bin/mediaclient --start");
    QThread::msleep(3000);

    ui->setupUi(this);

    ui->warn_no_dab_list->setVisible(false);

    //open both station lists #############################################################################################################
    //DAB
    QFile file_dab(path_dab);

    if(!file_dab.open(QFile::ReadOnly | QFile::Text)){
        QMessageBox::warning(this,"no stationlist for DAB found","no stationlist for DAB found\nPlease hit scan in mode DAB first!");
        ui->warn_no_dab_list->setVisible(true);
        return;
    }

    //FM
    QFile file_fm(path_fm);

    if(!file_fm.open(QFile::ReadOnly | QFile::Text)){
        QMessageBox::warning(this,"no stationlist for FM found","no stationlist for FM found\nPlease hit scan in mode FM first!");
        ui->warn_no_dab_list->setVisible(true);
        return;
    }

    // start DAB default ##################################################################################################################

    tgl_state = "DAB";

    //qDebug() << "state: " << tgl_state;
    //qDebug() << "button: " << ui->tgl_dab_fm->text();
    QProcess::execute("/opt/bin/mediaclient -d /dev/dab0");
    QProcess::execute("/opt/bin/mediaclient -m DAB -g off");
    ui->ls_dab->setVisible(true);
    ui->ls_fm->setVisible(false);

    //enable tune button if line is selcted

//    int ind_marked = ui->ls_dab->currentRow();
//    if(ind_marked > 0){
//    ui->btn_tune->setEnabled(true);
//    }

    //show saved stations from dab in list

    //QFile file_dab(path_dab);
    //if(!file_dab.open(QFile::ReadOnly | QFile::Text)){
        //QMessageBox::warning(this,"..","keine datei gefunden");
        //ui->warn_no_dab_list->setVisible(true);
       // return;
    //}

    //fill lists ##########################################################################################################################
    //DAB
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

    MainWindow::dab_list();
    /*
    for(int i = 0; i < dab.size(); i++){
      ui->ls_dab->addItem(dab[i][1]);
    }
    */
    //FM
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
/*
    for(int i = 0; i < fm.size(); i++){
      ui->ls_fm->addItem(fm[i][0]);
    }
    */
    MainWindow::fm_list();
}


MainWindow::~MainWindow()
{
    //kill mediaclient on close, else sound goes on...
    QProcess::execute("/opt/bin/mediaclient --shutdown");
    QProcess::execute("/opt/bin/mediaclient -m DAB -g on");
    QProcess::execute("/opt/bin/mediaclient -m RADIO -g on");

    delete ui;
}

void MainWindow::on_tgl_dab_fm_clicked()
{
    //tgl_state = "FM";

    if(ui->tgl_dab_fm->text() == "FM"){
        ui->tgl_dab_fm->setText("DAB");
        tgl_state = "FM";
        ui->ls_dab->setVisible(false);
        ui->ls_fm->setVisible(true);
    } else {
        ui->tgl_dab_fm->setText("FM");
        tgl_state = "DAB";
        ui->ls_dab->setVisible(true);
        ui->ls_fm->setVisible(false);
    }
    //know which mode is selected (FM or DAB, default on start is DAB)

    qDebug() << "state: " << tgl_state;
    qDebug() << "button: " << ui->tgl_dab_fm->text();


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
            fr = fr.remove(0, 3);
            freq_avail.append(fr);


            //ui->listScan->addItem(fr);
        }
        qDebug() << "freq avail:" << freq_avail;

        //tune zu transponder

        //open file and append found transponders

        QFile file_dab(path_dab);
        if(!file_dab.open(QFile::WriteOnly | QFile::Text)){
            //QMessageBox::warning(this,"..","keine datei gefunden");
            ui->warn_no_dab_list->setVisible(true);
            return;
        }

        //clear file before writing
        //file_dab.resize(0);
        QTextStream out(&file_dab);

        //QString transponder = trans.at(0);
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

                          /*
                          QStringList one_line = line_dab_trans.split('\t');
                          QString one = one_line;
                          ui->listScan->addItem(one_line);
                          */

                          //QString line = in.readLine();
                          //if(!line.contains(delete_marked, Qt::CaseSensitive)){
                          //QString outline = line;

                          //if(service_name != "Service Name"){
                              out << freq_avail.at(i) << "," << service_name << "," << matched << "\n";
                              qDebug() << freq_avail.at(i) << service_name << matched;
                          //}

                           //clear line_dab_trans at the end, else only transponders from frequency #0 aer listed
                          line_dab_trans.clear();
                      }
                }
        }

        file_dab.flush();
        file_dab.close();
        //nochmal prüfen ob notwendig...
        if(!file_dab.open(QFile::ReadOnly | QFile::Text)){
            //QMessageBox::warning(this,"no stationfile found","Please hit scan first!");
            ui->warn_no_dab_list->setVisible(true);
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
/*
        for(int i = 0; i < dab.size(); i++){
          ui->ls_dab->addItem(dab[i][1]);
        }
        */
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
        //qDebug() << "line_count_fm" << line_count_fm;

        //open file to write found freq

        QFile file_fm(path_fm);
        if(!file_fm.open(QFile::WriteOnly | QFile::Text)){
            //QMessageBox::warning(this,"..","keine datei gefunden");
            //ui->warn_no_dab_list->setVisible(true);
            return;
        }
        QTextStream out(&file_fm);

        for(int i = 0; i < line_count_fm; i++){
            QString tmp = console_find_freq.readLine();
            //if(tmp != "" && tmp != "Scan completed" && tmp != "SCAN SETUP"){
            //QString match = "LOCKED";
            if(tmp.contains("LOCKED")){
                tmp.replace(" [LOCKED]", "000");
                float mhz = tmp.toFloat() / 1000000;
                //QString mhz_str = QString::number(mhz).right();
                tmp = "Station " + QString::number(i-1) + " " + QString::number(mhz) + " MHz," + tmp;

                out << tmp << "\n";
//                QStringList split_tmp = tmp.split(",");
//                qDebug() << "split_tmp:" << split_tmp;
//                fm_row.push_back(split_tmp.at(0));
//                fm_row.push_back(split_tmp.at(1));

//                fm.push_back(fm_row);
            }

            //qDebug() << "fm_row" << fm_row;
            //qDebug() << "fm" << fm;
            //qDebug() << "station found" << fm;

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
                   qDebug() << split_text_fm;

                       QVector<QString> fm_row;

                       fm_row.push_back(split_text_fm.at(0));
                       fm_row.push_back(split_text_fm.at(1));


                       fm.push_back(fm_row);
                       qDebug() << "fm scan" << fm;

                   }
        file_fm.close();

        //add frequencies to list #########################################################################################################
        /*
        for(int i = 0; i < fm.size(); i++){
          ui->ls_fm->addItem(fm[i][0]);
        }
        */
        MainWindow::fm_list();
    }
}    

void MainWindow::on_btn_tune_clicked()
{

    MainWindow::tune();
    //DAB tune

    /*

    if (tgl_state == "DAB"){

        int ind_marked = ui->ls_dab->currentRow();
        //QString tune_en = QString::number(ind_marked);
        if(ind_marked > -1){

            //start mediaclient in case it did not...
            QProcess::execute("/opt/bin/mediaclient --start");

            //get frequency and sid from selected item in list


            freq = dab[ind_marked][0];
            serv_id = dab[ind_marked][2];

            //1. tune to frequency
            QProcess::execute("/opt/bin/mediaclient -m DAB -f" + freq);
            //2. add service id
            //QString station = ui->listScanDAB->currentItem()->text();
            //QRegularExpression sid_re("[0][x][a-f0-9][a-f0-9][a-f0-9][a-f0-9]");
            //QRegularExpressionMatch match = sid_re.match(station);
            //QString matched;
            //if (match.hasMatch()) {
            //    matched = match.captured(0);
            //}

            QProcess::execute("/opt/bin/mediaclient -m DAB -f " + freq + " --sid " + serv_id);

            QProcess::execute("/opt/bin/mediaclient -m DAB -g off");

            //write to vector

            //write to list
        }
    }
    */

    //FM tune

    /*

    if (tgl_state == "FM"){

        int ind_marked = ui->ls_fm->currentRow();
        //QString tune_en = QString::number(ind_marked);
        if(ind_marked > -1){

            //start mediaclient in case it did not...
            QProcess::execute("/opt/bin/mediaclient --start");

            //get frequency and sid from selected item in list

            freq = fm[ind_marked][1];

            //1. tune to frequency
            //QProcess::execute("/opt/bin/mediaclient -m DAB -f" + freq);
            //2. add service id
            //QString station = ui->listScanDAB->currentItem()->text();
            //QRegularExpression sid_re("[0][x][a-f0-9][a-f0-9][a-f0-9][a-f0-9]");
            //QRegularExpressionMatch match = sid_re.match(station);
            //QString matched;
            //if (match.hasMatch()) {
            //    matched = match.captured(0);
            //}

            QProcess::execute("/opt/bin/mediaclient -m RADIO -f " + freq);

            QProcess::execute("/opt/bin/mediaclient -m RADIO -g off");

            //write to vector

            //write to list
        }
    }
    */
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

void MainWindow::on_btn_clear_clicked()
{
    MainWindow::testfunction();
}

//#########################################################################################################################################
//functions

void MainWindow::testfunction(){

    QMessageBox::warning(this,"no stationlist for DAB found","no stationlist for DAB found\nPlease hit scan in mode DAB first!");
}

void MainWindow::fm_list(){
    for(int i = 0; i < fm.size(); i++){
      ui->ls_fm->addItem(fm[i][0]);
    }
}

void MainWindow::dab_list(){
    for(int i = 0; i < dab.size(); i++){
      ui->ls_dab->addItem(dab[i][1]);
    }
}

void MainWindow::tune(){

    QString radio_dab_type;

    if (tgl_state == "FM"){

        radio_dab_type = "RADIO";
        int ind_marked = ui->ls_fm->currentRow();
        if(ind_marked > -1){

            freq = fm[ind_marked][1];
        }
    }

    if (tgl_state == "DAB"){

        radio_dab_type = "DAB";
        int ind_marked = ui->ls_dab->currentRow();
        if(ind_marked > -1){

            freq = dab[ind_marked][0];
            serv_id = dab[ind_marked][2];
        }
    }

    //start mediaclient in case it did not...
    QProcess::execute("/opt/bin/mediaclient --start");
    QProcess::execute("/opt/bin/mediaclient -m" + radio_dab_type + " -f" + freq);
    if(tgl_state == "DAB"){
        QProcess::execute("/opt/bin/mediaclient -m" + radio_dab_type + " -f " + freq + " --sid " + serv_id);
    }
    QProcess::execute("/opt/bin/mediaclient -m" + radio_dab_type + " -g off");
}