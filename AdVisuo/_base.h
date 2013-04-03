// _base.h

#pragma once

interface IAction;
typedef IAction *ANIM_HANDLE;

interface ILostDeviceObserver
{
	virtual void OnLostDevice() = 0;
	virtual void OnResetDevice() = 0;
};

interface IAnimationListener
{
	virtual int OnAnimationBegin(AVULONG nParam) = 0;
	virtual int OnAnimationTick(AVULONG nParam) = 0;
	virtual int OnAnimationEnd(AVULONG nParam) = 0;
};

