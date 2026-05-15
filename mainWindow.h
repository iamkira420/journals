/**
 * @file mainwindow.h
 * @brief Declaration of MainWindow — the application's single top-level window.
 *
 * Layout (mirrors the reference screenshot):
 *
 *   ┌─────────────────┬──────────────────────────────────────────┐
 *   │  Left panel     │                                          │
 *   │  ─────────────  │          QTableView                      │
 *   │  Author  [    ] │                                          │
 *   │  Year    [    ] │  (sortable, colour-coded rows)           │
 *   │  Title   [    ] │                                          │
 *   │  Journal [    ] │                                          │
 *   │  Volume  [    ] │                                          │
 *   │  Issue   [    ] │                                          │
 *   │  Pages   [    ] │                                          │
 *   │  [  Add  ]      │                                          │
 *   │  [Remove ]      │                                          │
 *   │  ─────────────  │                                          │
 *   │  [filter text]  │                                          │
 *   │  [field combo]  │                                          │
 *   │  [Find][Reset]  │                                          │
 *   └─────────────────┴──────────────────────────────────────────┘
 *
 * The left panel width is fixed; the table view expands to fill the rest.
 */

#pragma once

#include <QMainWindow>

// Forward declarations — avoids heavyweight includes in the header
class ArticleModel;
class ArticleFilterProxy;
class QTableView;
class QComboBox;
class QLineEdit;
class QSpinBox;
class QLabel;
class QPushButton;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

private slots:
    // ── Toolbar / left-panel actions ──────────────────────────────────────
    void onAdd();       ///< Reads input fields and calls model->addArticle()
    void onRemove();    ///< Removes the currently selected row(s)
    void onFind();      ///< Applies wildcard filter from m_filterEdit
    void onReset();     ///< Clears the filter; all rows visible again

private:
    // ── Setup helpers (called once from the constructor) ──────────────────
    void buildLayout();         ///< Constructs the full left+right layout
    QWidget *buildLeftPanel();  ///< Returns the completed left-panel widget
    void buildTableView();      ///< Creates and wires up the QTableView
    void seedSampleData();      ///< Populates a handful of demo rows

    // ── Utility ───────────────────────────────────────────────────────────
    void clearInputs();         ///< Resets all input fields to defaults
    void showStatusMessage(const QString &msg, int timeoutMs = 3000);

    // ── Model / view stack ────────────────────────────────────────────────
    ArticleModel       *m_model = nullptr;  ///< Source data model
    ArticleFilterProxy *m_proxy = nullptr;  ///< Sort + filter proxy
    QTableView         *m_view  = nullptr;  ///< The visible table

    // ── Left-panel input widgets ──────────────────────────────────────────
    QLineEdit *m_authorEdit  = nullptr;
    QSpinBox  *m_yearSpin    = nullptr;
    QLineEdit *m_titleEdit   = nullptr;
    QLineEdit *m_journalEdit = nullptr;
    QSpinBox  *m_volumeSpin  = nullptr;
    QSpinBox  *m_issueSpin   = nullptr;
    QLineEdit *m_pagesEdit   = nullptr;

    // ── Filter widgets ────────────────────────────────────────────────────
    QLineEdit *m_filterEdit  = nullptr;  ///< Wildcard pattern input
    QComboBox *m_filterField = nullptr;  ///< Which field to filter on
};
