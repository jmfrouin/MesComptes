#include "Transaction.h"

Transaction::Transaction()
    : mId(-1), mDate(wxDateTime::Now()), mLibelle(""), mSomme(0.0),
      mPointee(false), mType("") {}

Transaction::Transaction(int id, const wxDateTime& date, const std::string& libelle,
                         double somme, bool pointee, const std::string& type)
    : mId(id), mDate(date), mLibelle(libelle), mSomme(somme),
      mPointee(pointee), mType(type) {}