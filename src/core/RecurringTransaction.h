//
// Created by Jean-Michel Frouin on 01/11/2025.
//

#ifndef RECURRINGTRANSACTION_H
#define RECURRINGTRANSACTION_H

#include <string>
#include <wx/datetime.h>

enum class RecurrenceType {
    DAILY,      // Quotidien
    WEEKLY,     // Hebdomadaire
    MONTHLY,    // Mensuel
    YEARLY      // Annuel
};

class RecurringTransaction {
public:
    RecurringTransaction();
    RecurringTransaction(int id, const std::string& libelle, double somme,
                        const std::string& type, RecurrenceType recurrence,
                        const wxDateTime& startDate, const wxDateTime& endDate,
                        int dayOfMonth = 1, bool active = true);

    // Getters
    int GetId() const { return mId; }
    std::string GetLibelle() const { return mLibelle; }
    double GetSomme() const { return mSomme; }
    std::string GetType() const { return mType; }
    RecurrenceType GetRecurrence() const { return mRecurrence; }
    wxDateTime GetStartDate() const { return mStartDate; }
    wxDateTime GetEndDate() const { return mEndDate; }
    wxDateTime GetLastExecuted() const { return mLastExecuted; }
    int GetDayOfMonth() const { return mDayOfMonth; }
    bool IsActive() const { return mActive; }

    // Setters
    void SetId(int id) { mId = id; }
    void SetLibelle(const std::string& libelle) { mLibelle = libelle; }
    void SetSomme(double somme) { mSomme = somme; }
    void SetType(const std::string& type) { mType = type; }
    void SetRecurrence(RecurrenceType recurrence) { mRecurrence = recurrence; }
    void SetStartDate(const wxDateTime& date) { mStartDate = date; }
    void SetEndDate(const wxDateTime& date) { mEndDate = date; }
    void SetLastExecuted(const wxDateTime& date) { mLastExecuted = date; }
    void SetDayOfMonth(int day) { mDayOfMonth = day; }
    void SetActive(bool active) { mActive = active; }

    // Calcule la prochaine date d'exécution
    wxDateTime GetNextExecutionDate() const;

    // Vérifie si la transaction doit être exécutée aujourd'hui
    bool ShouldExecuteToday() const;

private:
    int mId;
    std::string mLibelle;
    double mSomme;
    std::string mType;
    RecurrenceType mRecurrence;
    wxDateTime mStartDate;
    wxDateTime mEndDate;  // Date de fin (peut être invalide pour illimité)
    wxDateTime mLastExecuted;
    int mDayOfMonth;  // Pour les récurrences mensuelles (1-31)
    bool mActive;
};

#endif // RECURRINGTRANSACTION_H