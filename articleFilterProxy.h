/**
 * @author Lutho Mboniswa
 * @date 15 May 2026
 * @brief Declaration of ArticleFilterProxy — wildcard filter + numeric sort proxy.
 *
 * ArticleFilterProxy subclasses QSortFilterProxyModel and provides:
 *
 *  1. Column-specific wildcard filtering — the user picks a field (any column except Pages) and types a pattern; 
 *    only rows whose chosen column matches the wildcard are shown.
 *
 *  2. Correct numeric sorting for Year, Volume, and Issue columns — QSortFilterProxyModel sorts lexicographically by default, 
 *    which would put "9" after "10".  We override lessThan() to use the integer stored in Qt::UserRole for those columns.
 */

#pragma once

#include <QSortFilterProxyModel>

class ArticleFilterProxy : public QSortFilterProxyModel
{
    Q_OBJECT

    public:
        explicit ArticleFilterProxy(QObject *parent = nullptr);

        /**
         * @brief Sets the active filter.
         * @param pattern  Wildcard pattern (e.g. "Smith*", "*nano*"). An empty string matches everything (effectively clears the filter).
         * @param column   The source-model column index to filter on. Pass -1 to search across all filterable columns.
         */
        void setFilter(const QString &pattern, int column);

        /**
         * @brief Clears the active filter so all rows are visible again.
         */
        void clearFilter();

    protected:
        /**
         * @brief Decides whether a given source row should be visible.
         * Called by the framework whenever the proxy needs to refresh visibility.
         * Returns true if the row passes the current wildcard filter (or if no filter is active).
         */
        bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;

        /**
         * @brief Custom less-than comparer for sorting.
         * Uses Qt::UserRole integers for Year, Volume, and Issue columns so numeric ordering is correct.
         */
        bool lessThan(const QModelIndex &left, const QModelIndex &right) const override;

    private:
        QString m_pattern;      ///< Current wildcard pattern (may be empty)
        int     m_column;       ///< Column to filter on; -1 = all columns
        bool    m_active;       ///< Whether a filter is currently applied
};