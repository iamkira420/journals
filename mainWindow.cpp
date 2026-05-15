/**
 * @file mainWindow.cpp
 * @brief Implementation of MainWindow.
 *
 * Wires together ArticleModel, ArticleFilterProxy, and the UI widgets
 * described in mainWindow.h.
 */

#include "mainWindow.h"
#include "articleModel.h"
#include "articleFilterProxy.h"

#include <QTableView>
#include <QHeaderView>
#include <QLineEdit>
#include <QSpinBox>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QWidget>
#include <QFrame>
#include <QSplitter>
#include <QStatusBar>
#include <QDate>
#include <QMessageBox>
#include <QItemSelectionModel>
#include <QFont>
#include <QSizePolicy>

// ─────────────────────────────────────────────────────────────────────────────
// Construction
// ─────────────────────────────────────────────────────────────────────────────

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    // Create model and proxy first — widgets may need them during layout
    m_model = new ArticleModel(this);
    m_proxy = new ArticleFilterProxy(this);
    m_proxy->setSourceModel(m_model);

    buildLayout();
    seedSampleData();

    // Ready message in status bar
    statusBar()->showMessage("Ready  •  Double-click a cell to edit in-place");
}

// ─────────────────────────────────────────────────────────────────────────────
// Layout construction
// ─────────────────────────────────────────────────────────────────────────────

void MainWindow::buildLayout()
{
    // ── Central splitter: left panel | table ─────────────────────────────
    auto *splitter = new QSplitter(Qt::Horizontal, this);
    splitter->setHandleWidth(4);

    // Left panel is a fixed-width widget; table fills the rest
    QWidget *leftPanel = buildLeftPanel();
    leftPanel->setMinimumWidth(170);
    leftPanel->setMaximumWidth(210);

    buildTableView();   // populates m_view

    splitter->addWidget(leftPanel);
    splitter->addWidget(m_view);
    splitter->setStretchFactor(0, 0);   // left panel does not stretch
    splitter->setStretchFactor(1, 1);   // table view takes all extra space

    setCentralWidget(splitter);
}

QWidget *MainWindow::buildLeftPanel()
{
    auto *panel = new QWidget(this);
    auto *outer = new QVBoxLayout(panel);
    outer->setContentsMargins(8, 8, 8, 8);
    outer->setSpacing(6);

    // ── Input fields (QFormLayout for label-widget pairs) ─────────────────
    auto *formGroup = new QGroupBox("Article Details", panel);
    auto *form      = new QFormLayout(formGroup);
    form->setRowWrapPolicy(QFormLayout::WrapAllRows);
    form->setVerticalSpacing(4);

    // Author
    m_authorEdit = new QLineEdit(formGroup);
    m_authorEdit->setPlaceholderText("e.g. Smith, J.");
    form->addRow("Author", m_authorEdit);

    // Year — constrained to [1000, current year]
    m_yearSpin = new QSpinBox(formGroup);
    m_yearSpin->setRange(1000, QDate::currentDate().year());
    m_yearSpin->setValue(QDate::currentDate().year());  // sensible default
    form->addRow("Year", m_yearSpin);

    // Title
    m_titleEdit = new QLineEdit(formGroup);
    m_titleEdit->setPlaceholderText("Article title");
    form->addRow("Title", m_titleEdit);

    // Journal name
    m_journalEdit = new QLineEdit(formGroup);
    m_journalEdit->setPlaceholderText("Journal name");
    form->addRow("Journal", m_journalEdit);

    // Volume
    m_volumeSpin = new QSpinBox(formGroup);
    m_volumeSpin->setRange(0, 9999);
    form->addRow("Volume", m_volumeSpin);

    // Issue
    m_issueSpin = new QSpinBox(formGroup);
    m_issueSpin->setRange(0, 9999);
    form->addRow("Issue", m_issueSpin);

    // Pages (free text — "12-25", "i–xii", etc.)
    m_pagesEdit = new QLineEdit(formGroup);
    m_pagesEdit->setPlaceholderText("e.g. 123-145");
    form->addRow("Pages", m_pagesEdit);

    outer->addWidget(formGroup);

    // ── Add / Remove buttons ──────────────────────────────────────────────
    auto *addBtn    = new QPushButton("Add",    panel);
    auto *removeBtn = new QPushButton("Remove", panel);

    // Give the Add button a slightly more prominent look
    QFont boldFont = addBtn->font();
    boldFont.setBold(true);
    addBtn->setFont(boldFont);

    outer->addWidget(addBtn);
    outer->addWidget(removeBtn);

    // ── Separator ─────────────────────────────────────────────────────────
    auto *line = new QFrame(panel);
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);
    outer->addWidget(line);

    // ── Filter group ──────────────────────────────────────────────────────
    auto *filterGroup = new QGroupBox("Filter", panel);
    auto *filterVBox  = new QVBoxLayout(filterGroup);
    filterVBox->setSpacing(4);

    // Wildcard text input
    m_filterEdit = new QLineEdit(filterGroup);
    m_filterEdit->setPlaceholderText("Wildcard: e.g. Smith*");
    filterVBox->addWidget(m_filterEdit);

    // Field selector combo
    m_filterField = new QComboBox(filterGroup);
    // Items must match column indices in ArticleModel::Column.
    // Pages is excluded per the spec — it is not added here.
    m_filterField->addItem("Author",  ArticleModel::Col_Author);
    m_filterField->addItem("Year",    ArticleModel::Col_Year);
    m_filterField->addItem("Title",   ArticleModel::Col_Title);
    m_filterField->addItem("Journal", ArticleModel::Col_Journal);
    m_filterField->addItem("Volume",  ArticleModel::Col_Volume);
    m_filterField->addItem("Issue",   ArticleModel::Col_Issue);
    filterVBox->addWidget(m_filterField);

    // Find / Reset buttons side by side
    auto *findBtn  = new QPushButton("Find",  filterGroup);
    auto *resetBtn = new QPushButton("Reset", filterGroup);
    auto *btnRow   = new QHBoxLayout();
    btnRow->addWidget(findBtn);
    btnRow->addWidget(resetBtn);
    filterVBox->addLayout(btnRow);

    outer->addWidget(filterGroup);

    // Push everything to the top
    outer->addStretch();

    // ── Wire up signals ───────────────────────────────────────────────────
    connect(addBtn,    &QPushButton::clicked, this, &MainWindow::onAdd);
    connect(removeBtn, &QPushButton::clicked, this, &MainWindow::onRemove);
    connect(findBtn,   &QPushButton::clicked, this, &MainWindow::onFind);
    connect(resetBtn,  &QPushButton::clicked, this, &MainWindow::onReset);

    // Allow pressing Enter in the filter box to trigger the search
    connect(m_filterEdit, &QLineEdit::returnPressed, this, &MainWindow::onFind);

    return panel;
}

void MainWindow::buildTableView()
{
    m_view = new QTableView(this);
    m_view->setModel(m_proxy);  // View sees the proxy (sorted + filtered)

    // ── Behaviour ─────────────────────────────────────────────────────────
    m_view->setSortingEnabled(true);                    // click headers to sort
    m_view->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_view->setSelectionMode(QAbstractItemView::ExtendedSelection); // Ctrl/Shift select
    m_view->setEditTriggers(QAbstractItemView::DoubleClicked |
                             QAbstractItemView::SelectedClicked);
    m_view->setAlternatingRowColors(false);             // we handle colour ourselves
    m_view->setWordWrap(false);
    m_view->setShowGrid(true);

    // ── Header tweaks ─────────────────────────────────────────────────────
    QHeaderView *hh = m_view->horizontalHeader();
    hh->setSectionResizeMode(ArticleModel::Col_Author,  QHeaderView::Stretch);
    hh->setSectionResizeMode(ArticleModel::Col_Title,   QHeaderView::Stretch);
    hh->setSectionResizeMode(ArticleModel::Col_Journal, QHeaderView::Stretch);
    hh->setSectionResizeMode(ArticleModel::Col_Year,    QHeaderView::ResizeToContents);
    hh->setSectionResizeMode(ArticleModel::Col_Volume,  QHeaderView::ResizeToContents);
    hh->setSectionResizeMode(ArticleModel::Col_Issue,   QHeaderView::ResizeToContents);
    hh->setSectionResizeMode(ArticleModel::Col_Pages,   QHeaderView::ResizeToContents);
    hh->setSortIndicatorShown(true);
    hh->setSortIndicator(ArticleModel::Col_Author, Qt::AscendingOrder);

    // Hide the vertical header (row numbers) for a cleaner look
    m_view->verticalHeader()->setVisible(false);
    m_view->verticalHeader()->setDefaultSectionSize(24);

    // Default sort: by Author ascending
    m_proxy->sort(ArticleModel::Col_Author, Qt::AscendingOrder);
}

// ─────────────────────────────────────────────────────────────────────────────
// Slot implementations
// ─────────────────────────────────────────────────────────────────────────────

void MainWindow::onAdd()
{
    // ── Basic non-empty validation ─────────────────────────────────────────
    if (m_authorEdit->text().trimmed().isEmpty() ||
        m_titleEdit->text().trimmed().isEmpty()  ||
        m_journalEdit->text().trimmed().isEmpty())
    {
        QMessageBox::warning(this, "Missing Data",
                             "Author, Title, and Journal fields cannot be empty.");
        return;
    }

    // ── Attempt to add (model will validate the year) ─────────────────────
    const bool ok = m_model->addArticle(
        m_authorEdit->text().trimmed(),
        m_yearSpin->value(),
        m_titleEdit->text().trimmed(),
        m_journalEdit->text().trimmed(),
        m_volumeSpin->value(),
        m_issueSpin->value(),
        m_pagesEdit->text().trimmed()
    );

    if (ok) {
        showStatusMessage(QString("Article by \"%1\" added successfully.")
                              .arg(m_authorEdit->text().trimmed()));
        clearInputs();
    }
    // If !ok, the model already showed a QMessageBox explaining why.
}

void MainWindow::onRemove()
{
    QItemSelectionModel *sel = m_view->selectionModel();
    if (!sel->hasSelection()) {
        QMessageBox::information(this, "No Selection",
                                 "Please select one or more rows to remove.");
        return;
    }

    // Collect unique source rows (proxy rows map to source rows)
    // We must remove from highest row index downward to avoid shifting indices.
    QList<int> sourceRows;
    const QModelIndexList selected = sel->selectedRows();
    for (const QModelIndex &proxyIdx : selected) {
        const QModelIndex srcIdx = m_proxy->mapToSource(proxyIdx);
        sourceRows.append(srcIdx.row());
    }

    // Sort descending so removal doesn't invalidate earlier indices
    std::sort(sourceRows.begin(), sourceRows.end(), std::greater<int>());
    // Remove duplicates (selectedRows() should not give duplicates, but be safe)
    sourceRows.erase(std::unique(sourceRows.begin(), sourceRows.end()),
                     sourceRows.end());

    const int count = sourceRows.size();
    const auto answer = QMessageBox::question(
        this, "Confirm Removal",
        QString("Remove %1 selected article(s)?").arg(count),
        QMessageBox::Yes | QMessageBox::No
    );
    if (answer != QMessageBox::Yes)
        return;

    for (int row : sourceRows)
        m_model->removeRow(row);

    showStatusMessage(QString("%1 article(s) removed.").arg(count));
}

void MainWindow::onFind()
{
    const QString pattern = m_filterEdit->text().trimmed();
    if (pattern.isEmpty()) {
        // Nothing typed — treat as a reset
        m_proxy->clearFilter();
        showStatusMessage("Filter cleared — showing all records.");
        return;
    }

    // Retrieve the column index stored as item data in the combo
    const int column = m_filterField->currentData().toInt();
    m_proxy->setFilter(pattern, column);

    const int visible = m_proxy->rowCount();
    showStatusMessage(QString("Filter applied — %1 record(s) match \"%2\" in %3.")
                          .arg(visible)
                          .arg(pattern)
                          .arg(m_filterField->currentText()));
}

void MainWindow::onReset()
{
    m_filterEdit->clear();
    m_proxy->clearFilter();
    showStatusMessage("Filter cleared — showing all records.");
}

// ─────────────────────────────────────────────────────────────────────────────
// Private helpers
// ─────────────────────────────────────────────────────────────────────────────

void MainWindow::clearInputs()
{
    m_authorEdit->clear();
    m_titleEdit->clear();
    m_journalEdit->clear();
    m_pagesEdit->clear();
    m_yearSpin->setValue(QDate::currentDate().year());
    m_volumeSpin->setValue(0);
    m_issueSpin->setValue(0);

    // Return focus to the first field for quick data entry
    m_authorEdit->setFocus();
}

void MainWindow::showStatusMessage(const QString &msg, int timeoutMs)
{
    statusBar()->showMessage(msg, timeoutMs);
}

void MainWindow::seedSampleData()
{
    // Pre-populate with representative articles spanning different colour bands.
    // Current year is obtained dynamically so colours stay correct in future.
    const int cur = QDate::currentDate().year();

    // Green rows (within last 5 years)
    m_model->addArticle("Zhang, L. & Wang, H.",  cur - 1,
                         "Deep Learning for Protein Folding",
                         "Nature Methods",          19, 3, "301-315");

    m_model->addArticle("Müller, K. et al.",      cur - 3,
                         "Quantum Error Correction at Scale",
                         "Physical Review Letters", 131, 7, "071401");

    m_model->addArticle("Okonkwo, A.",             cur - 4,
                         "Sub-Saharan Climate Modelling",
                         "Climate Dynamics",        61, 2, "889-904");

    // Neutral rows (between 5 and 10 years ago)
    m_model->addArticle("Hernandez, M. & Liu, Y.", cur - 7,
                         "Graph Neural Networks: A Review",
                         "IEEE Trans. Neural Netw.", 34, 1, "44-65");

    m_model->addArticle("Petrov, S.",               cur - 8,
                         "Topological Insulators in 2D",
                         "Science",                 341, 6148, "153-157");

    // Red rows (older than 10 years)
    m_model->addArticle("Smith, J. & Brown, T.",   cur - 12,
                         "PageRank Algorithm Analysis",
                         "ACM Comput. Surv.",        45, 4, "1-32");

    m_model->addArticle("Nakamura, R.",             cur - 15,
                         "Blue LED Efficiency Breakthroughs",
                         "Applied Physics Letters",  88, 10, "101106");
}