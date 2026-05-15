/**
 * @author Lutho Mboniswa
 * @date 15 May 2026
 * @brief Declaration of ArticleModel — the core data model for journal articles.
 *
 * ArticleModel subclasses QStandardItemModel and adds:
 *   - Named column indices (Col_Author - Col_Pages)
 *   - Year validation: rejects years later than the current year
 *   - Row colour highlighting:
 *       • Red   — article is older than 10 years
 *       • Green — article is within the last 5 years
 *       • No tint — everything in between
 *
 * The current year is determined dynamically via QDate::currentDate() so the application never needs to be updated for the calendar to roll over.
 */

#pragma once

#include <QStandardItemModel>

class ArticleModel : public QStandardItemModel
{
    Q_OBJECT

    public:
    // Column index constants   
        enum Column {
            Col_Author  = 0,
            Col_Year    = 1,
            Col_Title   = 2,
            Col_Journal = 3,
            Col_Volume  = 4,
            Col_Issue   = 5,
            Col_Pages   = 6,
            Col_COUNT   = 7   // Total number of columns — keep last.
        };

        explicit ArticleModel(QObject *parent = nullptr);

    /**
     * @brief Adds a new article row to the model.
     *
     * Validates that @p year is not later than the current calendar year.
     * If validation fails the row is NOT added and the function returns false.
     *
     * @param author   Author name(s)
     * @param year     Year of publication (must be ≤ current year)
     * @param title    Article title
     * @param journal  Journal name
     * @param volume   Volume number
     * @param issue    Issue number
     * @param pages    Page range (e.g. "123-145")
     * @return true if the row was added successfully; false if year is invalid.
     */
    bool addArticle(const QString &author, int year, const QString &title, const QString &journal, int volume, int issue, const QString &pages);

    //  QAbstractItemModel overrides 

    /**
     * @brief Returns data for the given index and role.
     *
     * Overridden to inject Qt::BackgroundRole colouring based on the year stored in Col_Year for the same row.  
     * All other roles are delegated to the base class.
     */
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    /**
     * @brief Handles in-place editing of cells.
     *
     * Overridden so that:
     *   - Edits to Col_Year are validated (no future year allowed).
     *   - After any edit the row colour is refreshed automatically.
     */
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

    private:
        /**
         * @brief Computes the background colour for a given publication year.
         *
         * Rules (evaluated against QDate::currentDate().year()):
         *   year < currentYear - 10  → red   (#FFCCCC)
         *   year >= currentYear - 5  → green (#CCFFCC)
         *   otherwise                → invalid QColor (no tint)
         *
         * @param year  The publication year to evaluate.
         * @return A QColor appropriate for the row background, or QColor() for none.
         */
        
        static QColor colourForYear(int year);
};