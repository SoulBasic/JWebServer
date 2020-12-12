#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "http_connector.hpp"
#include <chrono>
#include <thread>
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_sendButton_clicked()
{
	QString url = ui->headerEdit->toPlainText();
	QString body = ui->bodyEdit->toPlainText();
	qDebug(body.toStdString().c_str());
	std::string buffer;
	HttpConnector hc("192.168.199.132", url.toStdString(),buffer, 2324);
	if (!hc.init())
	{
		qDebug("%s", hc.getErrno().c_str());
		return;
	}
	qDebug("connection build");
	if (!hc.request(body.toStdString()))
	{
		qDebug("%s", hc.getErrno().c_str());
		return;
	}
	qDebug("requested");
	std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	ui->responseEdit->setPlainText(QString(buffer.c_str()));
}
