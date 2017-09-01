#pragma once

class CNonCopyable
{
protected:
    CNonCopyable(){}
    ~CNonCopyable(){}
private:
    CNonCopyable(const CNonCopyable&);
    const CNonCopyable& operator=(const CNonCopyable&);
};