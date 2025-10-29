//
// Created by Jean-Michel Frouin on 27/10/2025.
//

#ifndef TRANSACTION_H
#define TRANSACTION_H

#include <string>
#include <wx/datetime.h>

class Transaction {
    public:
        Transaction();
        Transaction(int id, const wxDateTime& date, const std::string& libelle,
                    double somme, bool pointee, const std::string& type);

        // Getters
        int GetId() const { return mId; }
        wxDateTime GetDate() const { return mDate; }
        std::string GetLibelle() const { return mLibelle; }
        double GetSomme() const { return mSomme; }
        bool IsPointee() const { return mPointee; }
        std::string GetType() const { return mType; }
        wxDateTime GetDatePointee() const { return mDatePointee; }


        // Setters
        void SetId(int id) { mId = id; }
        void SetDate(const wxDateTime& date) { mDate = date; }
        void SetLibelle(const std::string& libelle) { mLibelle = libelle; }
        void SetSomme(double somme) { mSomme = somme; }
        void SetPointee(bool pointee) { mPointee = pointee; }
        void SetType(const std::string& type) { mType = type; }
        void SetDatePointee(const wxDateTime& datePointee) { mDatePointee = datePointee; }

    private:
        int mId;
        wxDateTime mDate;
        std::string mLibelle;
        double mSomme;
        bool mPointee;
        std::string mType;
        wxDateTime mDatePointee;
};

#endif // TRANSACTION_H