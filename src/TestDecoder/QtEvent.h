#pragma once
#include <QEvent>
//#include <QCoreApplication>
class QtEvent : public QEvent
{
public:
	enum Type
	{
		SetCanvas = QEvent::User + 1,
		Event2 = QEvent::User + 2,
	};
	QtEvent(Type type) : QEvent(QEvent::Type(type)) {};
};

//QtEvent * _event = new QtEvent(QtEvent::Event1);
//QCoreApplication::postEvent(this, _event);
//bool ClassName::event(QEvent *event)
//{
//	if (event->type() == QtEvent::Event1)
//	{
//		return true;
//	}
//	return QThread::event(event);
//}