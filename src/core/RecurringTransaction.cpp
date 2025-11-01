//
// Created by Jean-Michel Frouin on 01/11/2025.
//

#include "RecurringTransaction.h"
#include <algorithm>

RecurringTransaction::RecurringTransaction()
    : mId(0), mLibelle(""), mSomme(0.0), mType(""),
      mRecurrence(RecurrenceType::MONTHLY), mDayOfMonth(1), mActive(true) {
}

RecurringTransaction::RecurringTransaction(int id, const std::string& libelle,
                                          double somme, const std::string& type,
                                          RecurrenceType recurrence,
                                          const wxDateTime& startDate,
                                          const wxDateTime& endDate,
                                          int dayOfMonth, bool active)
    : mId(id), mLibelle(libelle), mSomme(somme), mType(type),
      mRecurrence(recurrence), mStartDate(startDate), mEndDate(endDate),
      mDayOfMonth(dayOfMonth), mActive(active) {
}

wxDateTime RecurringTransaction::GetNextExecutionDate() const {
    wxDateTime nextDate;

    if (!mLastExecuted.IsValid()) {
        nextDate = mStartDate;
    } else {
        nextDate = mLastExecuted;

        switch (mRecurrence) {
            case RecurrenceType::DAILY:
                nextDate.Add(wxDateSpan::Day());
                break;

            case RecurrenceType::WEEKLY:
                nextDate.Add(wxDateSpan::Week());
                break;

            case RecurrenceType::MONTHLY:
                nextDate.Add(wxDateSpan::Month());
                nextDate.SetDay(std::min(mDayOfMonth,
                            static_cast<int>(wxDateTime::GetNumberOfDays(nextDate.GetMonth(), nextDate.GetYear()))));
                break;

            case RecurrenceType::YEARLY:
                nextDate.Add(wxDateSpan::Year());
                break;
        }
    }

    return nextDate;
}

bool RecurringTransaction::ShouldExecuteToday() const {
    if (!mActive) {
        return false;
    }

    wxDateTime today = wxDateTime::Today();
    wxDateTime nextExec = GetNextExecutionDate();

    // Vérifier si la date de fin n'est pas dépassée
    if (mEndDate.IsValid() && today > mEndDate) {
        return false;
    }

    // Vérifier si on est avant la date de début
    if (today < mStartDate) {
        return false;
    }

    // Vérifier si on doit exécuter aujourd'hui
    return nextExec.IsSameDate(today) || nextExec < today;
}