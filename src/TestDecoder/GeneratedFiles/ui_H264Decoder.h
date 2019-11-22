/********************************************************************************
** Form generated from reading UI file 'H264Decoder.ui'
**
** Created by: Qt User Interface Compiler version 5.9.8
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_H264DECODER_H
#define UI_H264DECODER_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_H264DecoderClass
{
public:
    QPushButton *CreateSession;
    QPushButton *DeleteSession;
    QPushButton *GetFree;
    QPushButton *TryBitStreamDemux;
    QPushButton *BeginDecode;
    QPushButton *StopDecode;
    QPushButton *TryNetStreamDemux;
    QLineEdit *filePath;
    QPushButton *OpenFile;
    QPushButton *StartSendData;
    QPushButton *EndSendData;

    void setupUi(QWidget *H264DecoderClass)
    {
        if (H264DecoderClass->objectName().isEmpty())
            H264DecoderClass->setObjectName(QStringLiteral("H264DecoderClass"));
        H264DecoderClass->resize(600, 400);
        CreateSession = new QPushButton(H264DecoderClass);
        CreateSession->setObjectName(QStringLiteral("CreateSession"));
        CreateSession->setGeometry(QRect(52, 100, 121, 28));
        DeleteSession = new QPushButton(H264DecoderClass);
        DeleteSession->setObjectName(QStringLiteral("DeleteSession"));
        DeleteSession->setGeometry(QRect(52, 180, 121, 28));
        GetFree = new QPushButton(H264DecoderClass);
        GetFree->setObjectName(QStringLiteral("GetFree"));
        GetFree->setGeometry(QRect(70, 250, 93, 28));
        TryBitStreamDemux = new QPushButton(H264DecoderClass);
        TryBitStreamDemux->setObjectName(QStringLiteral("TryBitStreamDemux"));
        TryBitStreamDemux->setGeometry(QRect(190, 100, 161, 28));
        BeginDecode = new QPushButton(H264DecoderClass);
        BeginDecode->setObjectName(QStringLiteral("BeginDecode"));
        BeginDecode->setGeometry(QRect(210, 190, 111, 28));
        StopDecode = new QPushButton(H264DecoderClass);
        StopDecode->setObjectName(QStringLiteral("StopDecode"));
        StopDecode->setGeometry(QRect(210, 250, 111, 28));
        TryNetStreamDemux = new QPushButton(H264DecoderClass);
        TryNetStreamDemux->setObjectName(QStringLiteral("TryNetStreamDemux"));
        TryNetStreamDemux->setGeometry(QRect(190, 140, 161, 28));
        filePath = new QLineEdit(H264DecoderClass);
        filePath->setObjectName(QStringLiteral("filePath"));
        filePath->setGeometry(QRect(50, 40, 381, 20));
        OpenFile = new QPushButton(H264DecoderClass);
        OpenFile->setObjectName(QStringLiteral("OpenFile"));
        OpenFile->setGeometry(QRect(450, 40, 75, 23));
        StartSendData = new QPushButton(H264DecoderClass);
        StartSendData->setObjectName(QStringLiteral("StartSendData"));
        StartSendData->setGeometry(QRect(360, 100, 121, 23));
        EndSendData = new QPushButton(H264DecoderClass);
        EndSendData->setObjectName(QStringLiteral("EndSendData"));
        EndSendData->setGeometry(QRect(360, 130, 121, 23));

        retranslateUi(H264DecoderClass);

        QMetaObject::connectSlotsByName(H264DecoderClass);
    } // setupUi

    void retranslateUi(QWidget *H264DecoderClass)
    {
        H264DecoderClass->setWindowTitle(QApplication::translate("H264DecoderClass", "H264Decoder", Q_NULLPTR));
        CreateSession->setText(QApplication::translate("H264DecoderClass", "CreateSession", Q_NULLPTR));
        DeleteSession->setText(QApplication::translate("H264DecoderClass", "DeleteSession", Q_NULLPTR));
        GetFree->setText(QApplication::translate("H264DecoderClass", "GetFree", Q_NULLPTR));
        TryBitStreamDemux->setText(QApplication::translate("H264DecoderClass", "TryBitStreamDemux", Q_NULLPTR));
        BeginDecode->setText(QApplication::translate("H264DecoderClass", "BeginDecode", Q_NULLPTR));
        StopDecode->setText(QApplication::translate("H264DecoderClass", "StopDecode", Q_NULLPTR));
        TryNetStreamDemux->setText(QApplication::translate("H264DecoderClass", "TryNetStreamDemux", Q_NULLPTR));
        OpenFile->setText(QApplication::translate("H264DecoderClass", "OpenFile", Q_NULLPTR));
        StartSendData->setText(QApplication::translate("H264DecoderClass", "StartSendData", Q_NULLPTR));
        EndSendData->setText(QApplication::translate("H264DecoderClass", "EndSendData", Q_NULLPTR));
    } // retranslateUi

};

namespace Ui {
    class H264DecoderClass: public Ui_H264DecoderClass {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_H264DECODER_H
