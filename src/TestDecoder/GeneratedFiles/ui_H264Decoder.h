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
#include <QtWidgets/QPushButton>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_H264DecoderClass
{
public:
    QPushButton *createsession;
    QPushButton *deletesession;
    QPushButton *GetFree;
    QPushButton *trydemux;
    QPushButton *begindecode;
    QPushButton *stopdecode;
    QPushButton *trynetstreamdemux;

    void setupUi(QWidget *H264DecoderClass)
    {
        if (H264DecoderClass->objectName().isEmpty())
            H264DecoderClass->setObjectName(QStringLiteral("H264DecoderClass"));
        H264DecoderClass->resize(600, 400);
        createsession = new QPushButton(H264DecoderClass);
        createsession->setObjectName(QStringLiteral("createsession"));
        createsession->setGeometry(QRect(52, 100, 121, 28));
        deletesession = new QPushButton(H264DecoderClass);
        deletesession->setObjectName(QStringLiteral("deletesession"));
        deletesession->setGeometry(QRect(52, 180, 121, 28));
        GetFree = new QPushButton(H264DecoderClass);
        GetFree->setObjectName(QStringLiteral("GetFree"));
        GetFree->setGeometry(QRect(70, 250, 93, 28));
        trydemux = new QPushButton(H264DecoderClass);
        trydemux->setObjectName(QStringLiteral("trydemux"));
        trydemux->setGeometry(QRect(210, 100, 111, 28));
        begindecode = new QPushButton(H264DecoderClass);
        begindecode->setObjectName(QStringLiteral("begindecode"));
        begindecode->setGeometry(QRect(210, 180, 111, 28));
        stopdecode = new QPushButton(H264DecoderClass);
        stopdecode->setObjectName(QStringLiteral("stopdecode"));
        stopdecode->setGeometry(QRect(210, 250, 111, 28));
        trynetstreamdemux = new QPushButton(H264DecoderClass);
        trynetstreamdemux->setObjectName(QStringLiteral("trynetstreamdemux"));
        trynetstreamdemux->setGeometry(QRect(380, 100, 161, 28));

        retranslateUi(H264DecoderClass);

        QMetaObject::connectSlotsByName(H264DecoderClass);
    } // setupUi

    void retranslateUi(QWidget *H264DecoderClass)
    {
        H264DecoderClass->setWindowTitle(QApplication::translate("H264DecoderClass", "H264Decoder", Q_NULLPTR));
        createsession->setText(QApplication::translate("H264DecoderClass", "CreateSession", Q_NULLPTR));
        deletesession->setText(QApplication::translate("H264DecoderClass", "DeleteSession", Q_NULLPTR));
        GetFree->setText(QApplication::translate("H264DecoderClass", "GetFree", Q_NULLPTR));
        trydemux->setText(QApplication::translate("H264DecoderClass", "TryDemux", Q_NULLPTR));
        begindecode->setText(QApplication::translate("H264DecoderClass", "BeginDecode", Q_NULLPTR));
        stopdecode->setText(QApplication::translate("H264DecoderClass", "StopDecode", Q_NULLPTR));
        trynetstreamdemux->setText(QApplication::translate("H264DecoderClass", "TryNetStreamDemux", Q_NULLPTR));
    } // retranslateUi

};

namespace Ui {
    class H264DecoderClass: public Ui_H264DecoderClass {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_H264DECODER_H
