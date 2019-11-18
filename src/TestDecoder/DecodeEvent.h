#pragma once
#include <QEvent>
class DecodeEvent : public QEvent
{
public:
	enum Type
	{
		DemuxSuccess = QEvent::User + 1,
		CloseWidget = QEvent::User + 2,
	};
	DecodeEvent(Type tp) :QEvent((QEvent::Type)tp) {}
};