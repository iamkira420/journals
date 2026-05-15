/**
 * @file articlefilterproxy.cpp
 * @brief Implementation of ArticleFilterProxy.
 *
 * See articlefilterproxy.h for the full class description.
 */

#include "articleFilterProxy.h"
#include "articleModel.h"

#include <QRegularExpression>

// ─────────────────────────────────────────────────────────────────────────────
// Construction
// ─────────────────────────────────────────────────────────────────────────────

ArticleFilterProxy::ArticleFilterProxy(QObject *parent)
    : QSortFilterProxyModel(parent)
    , m_column(-1)
    , m_active(false)
{
    // Case-insensitive matching by default
    setSortCaseSensitivity(Qt::CaseInsensitive);
    setFilterCaseSensitivity(Qt::CaseInsensitive);
}

// ─────────────────────────────────────────────────────────────────────────────
// Public interface
// ─────────────────────────────────────────────────────────────────────────────

void ArticleFilterProxy::setFilter(const QString &pattern, int column)
{
    m_pattern = pattern.trimmed();
    m_column  = column;
    m_active  = !m_pattern.isEmpty();

    // Tell the framework to re-evaluate all rows
    invalidateFilter();
}

void ArticleFilterProxy::clearFilter()
{
    m_pattern.clear();
    m_column = -1;
    m_active = false;
    invalidateFilter();
}

// ─────────────────────────────────────────────────────────────────────────────
// Protected overrides
// ─────────────────────────────────────────────────────────────────────────────

bool ArticleFilterProxy::filterAcceptsRow(int sourceRow,
                                           const QModelIndex &sourceParent) const
{
    // No filter active → show every row
    if (!m_active)
        return true;

    // Convert the user's wildcard pattern to a QRegularExpression.
    // QRegularExpression::wildcardToRegularExpression handles * and ?
    // and anchors the pattern to the full string by default.
    const QString regexStr =
        QRegularExpression::wildcardToRegularExpression(m_pattern);
    const QRegularExpression re(regexStr, QRegularExpression::CaseInsensitiveOption);

    const QAbstractItemModel *src = sourceModel();

    if (m_column >= 0) {
        // ── Single-column filter ──────────────────────────────────────────
        const QModelIndex idx = src->index(sourceRow, m_column, sourceParent);
        const QString cellText = src->data(idx, Qt::DisplayRole).toString();
        return re.match(cellText).hasMatch();
    } else {
        // ── Search all filterable columns (everything except Pages) ───────
        // Pages == Col_Pages is excluded per the spec.
        for (int col = 0; col < ArticleModel::Col_COUNT; ++col) {
            if (col == ArticleModel::Col_Pages)
                continue;
            const QModelIndex idx = src->index(sourceRow, col, sourceParent);
            const QString cellText = src->data(idx, Qt::DisplayRole).toString();
            if (re.match(cellText).hasMatch())
                return true;
        }
        return false;
    }
}

bool ArticleFilterProxy::lessThan(const QModelIndex &left,
                                   const QModelIndex &right) const
{
    // For numeric columns, compare by the integer stored in Qt::UserRole
    // to avoid lexicographic errors ("9" > "10" as strings).
    switch (left.column()) {
    case ArticleModel::Col_Year:
    case ArticleModel::Col_Volume:
    case ArticleModel::Col_Issue: {
        const int lVal = sourceModel()->data(left,  Qt::UserRole).toInt();
        const int rVal = sourceModel()->data(right, Qt::UserRole).toInt();
        return lVal < rVal;
    }
    default:
        // Text columns: delegate to base class (locale-aware string compare)
        return QSortFilterProxyModel::lessThan(left, right);
    }
}