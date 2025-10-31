#include "Transaction.h"
#include <iostream>

Transaction::Transaction()
    : mId(-1), mDate(wxDateTime::Now()), mLibelle(""), mSomme(0.0),
      mPointee(false), mType("") { std::cout << "-" << mId << std::endl; }

Transaction::Transaction(int id, const wxDateTime& date, const std::string& libelle,
                         double somme, bool pointee, const std::string& type)
    : mId(id), mDate(date), mLibelle(libelle), mSomme(somme),
      mPointee(pointee), mType(type) { std::cout << "0-" << mId << std::endl;}

Transaction::Transaction(const Transaction& other)
    : mId(other.mId),
      mDate(other.mDate),
      mLibelle(other.mLibelle),
      mSomme(other.mSomme),
      mPointee(other.mPointee),
      mType(other.mType),
      mDatePointee(other.mDatePointee) {
    std::cout << "1-" << mId << std::endl;
}

Transaction& Transaction::operator=(const Transaction& other) {
    if (this != &other) {
        mId = other.mId;
        mDate = other.mDate;
        mLibelle = other.mLibelle;
        mSomme = other.mSomme;
        mPointee = other.mPointee;
        mType = other.mType;
        mDatePointee = other.mDatePointee;
    }
    std::cout << "2-" << mId << std::endl;
    return *this;
}