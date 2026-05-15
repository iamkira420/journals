/**
 * @file articlemodel.cpp
 * @brief Implementation of ArticleModel.
 *
 * See articlemodel.h for the full class description.
 */

#include "articleModel.h"

#include <QDate>
#include <QColor>
#include <QBrush>
#include <QMessageBox>

// ─────────────────────────────────────────────────────────────────────────────
// Construction
// ─────────────────────────────────────────────────────────────────────────────

ArticleModel::ArticleModel(QObject *parent)
    : QStandardItemModel(0, Col_COUNT, parent)
{
    // Set human-readable column headers
    setHorizontalHeaderLabels({
        "Author",
        "Year",
        "Title",
        "Journal",
        "Volume",
        "Issue",
        "Pages"
    });
}

// ─────────────────────────────────────────────────────────────────────────────
// Public interface
// ─────────────────────────────────────────────────────────────────────────────

bool ArticleModel::addArticle(const QString &author,
                               int            year,
                               const QString &title,
                               const QString &journal,
                               int            volume,
                               int            issue,
                               const QString &pages)
{
    // ── Year validation ───────────────────────────────────────────────────
    const int currentYear = QDate::currentDate().year();
    if (year > currentYear) {
        // Inform the user clearly; do not silently clamp the value.
        QMessageBox::warning(
            nullptr,
            "Invalid Year",
            QString("The publication year %1 is in the future.\n"
                    "Please enter a year no later than %2.")
                .arg(year)
                .arg(currentYear)
        );
        return false;   // Caller should keep the form open for correction.
    }

    // ── Build a row of QStandardItems ─────────────────────────────────────
    // We store Year, Volume, and Issue with Qt::UserRole as integers so that
    // numeric sorting works correctly (lexicographic "9" > "10" is wrong).

    auto *itemAuthor  = new QStandardItem(author);
    auto *itemYear    = new QStandardItem(QString::number(year));
    auto *itemTitle   = new QStandardItem(title);
    auto *itemJournal = new QStandardItem(journal);
    auto *itemVolume  = new QStandardItem(QString::number(volume));
    auto *itemIssue   = new QStandardItem(QString::number(issue));
    auto *itemPages   = new QStandardItem(pages);

    // Store integer values alongside display text for correct numeric sorting
    itemYear->setData(year,   Qt::UserRole);
    itemVolume->setData(volume, Qt::UserRole);
    itemIssue->setData(issue,  Qt::UserRole);

    // Make all cells editable (default) — setData() will enforce constraints
    appendRow({ itemAuthor, itemYear, itemTitle,
                itemJournal, itemVolume, itemIssue, itemPages });

    return true;
}

// ─────────────────────────────────────────────────────────────────────────────
// QAbstractItemModel overrides
// ─────────────────────────────────────────────────────────────────────────────

QVariant ArticleModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return {};

    // ── Background colouring ──────────────────────────────────────────────
    if (role == Qt::BackgroundRole) {
        // Read the year from the same row, Col_Year column
        const QModelIndex yearIndex = this->index(index.row(), Col_Year);
        // Use UserRole integer if available, otherwise parse the display string
        bool ok = false;
        int year = QStandardItemModel::data(yearIndex, Qt::UserRole).toInt(&ok);
        if (!ok)
            year = QStandardItemModel::data(yearIndex, Qt::DisplayRole).toString().toInt(&ok);

        if (ok) {
            const QColor c = colourForYear(year);
            if (c.isValid())
                return QBrush(c);
        }
        return {};  // No custom background
    }

    // ── Alignment: centre numeric columns ────────────────────────────────
    if (role == Qt::TextAlignmentRole) {
        switch (index.column()) {
        case Col_Year:
        case Col_Volume:
        case Col_Issue:
            return int(Qt::AlignCenter);
        default:
            break;
        }
    }

    // All other roles: delegate to base class
    return QStandardItemModel::data(index, role);
}

bool ArticleModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid() || role != Qt::EditRole)
        return QStandardItemModel::setData(index, value, role);

    // ── Validate year edits ───────────────────────────────────────────────
    if (index.column() == Col_Year) {
        bool ok = false;
        const int newYear = value.toInt(&ok);
        const int currentYear = QDate::currentDate().year();

        if (!ok || newYear > currentYear) {
            QMessageBox::warning(
                nullptr,
                "Invalid Year",
                QString("The year must be a whole number no later than %1.")
                    .arg(currentYear)
            );
            return false;   // Reject the edit — cell reverts to old value
        }

        // Accept: update both display text and UserRole integer
        QStandardItemModel::setData(index, QString::number(newYear), Qt::DisplayRole);
        QStandardItemModel::setData(index, newYear, Qt::UserRole);

        // Notify the view that the background of the whole row may have changed
        // by emitting dataChanged for every cell in the row.
        const QModelIndex rowStart = this->index(index.row(), 0);
        const QModelIndex rowEnd   = this->index(index.row(), Col_COUNT - 1);
        emit dataChanged(rowStart, rowEnd, { Qt::BackgroundRole });

        return true;
    }

    // ── Keep UserRole integers in sync for Volume / Issue ─────────────────
    if (index.column() == Col_Volume || index.column() == Col_Issue) {
        bool ok = false;
        const int intVal = value.toInt(&ok);
        if (ok) {
            QStandardItemModel::setData(index, QString::number(intVal), Qt::DisplayRole);
            QStandardItemModel::setData(index, intVal, Qt::UserRole);
            return true;
        }
    }

    return QStandardItemModel::setData(index, value, role);
}

// ─────────────────────────────────────────────────────────────────────────────
// Private helpers
// ─────────────────────────────────────────────────────────────────────────────

/*static*/
QColor ArticleModel::colourForYear(int year)
{
    const int current = QDate::currentDate().year();

    if (year < current - 10)
        return QColor(0xFF, 0xCC, 0xCC);    // Soft red  — older than 10 years

    if (year >= current - 5)
        return QColor(0xCC, 0xFF, 0xCC);    // Soft green — within last 5 years

    return {};   // Invalid colour == no tint (between 5 and 10 years ago)
}
