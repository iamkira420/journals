# JournalDB — Qt Journal Article Database

A desktop application built with **C++ and Qt 6** (compatible with Qt 5.15+)
that manages a database of academic journal article references, featuring
sortable columns, wildcard filtering, in-place editing, and automatic
colour-coding by publication age.

---

## Screenshot

```
┌─────────────────┬──────────────────────────────────────────────────────────┐
│ Article Details │  Author          Year  Title         Journal  Vol  Iss  Pg│
│ ─────────────── │  ─────────────────────────────────────────────────────── │
│ Author  [     ] │  Zhang, L. …     2024  Deep Learn…   Nature…  19   3    …│← green
│ Year    [2025 ] │  Müller, K. …    2022  Quantum Er…   Phys R…  131  7    …│← green
│ Title   [     ] │  Petrov, S.      2017  Topological…  Science  341  6148 …│
│ Journal [     ] │  Smith, J. …     2013  PageRank A…   ACM C…   45   4    …│← red
│ Volume  [     ] │  Nakamura, R.    2010  Blue LED E…   Appl P…  88   10   …│← red
│ Issue   [     ] │                                                            │
│ Pages   [     ] │                                                            │
│  [ Add ]        │                                                            │
│  [Remove]       │                                                            │
│ ─────────────── │                                                            │
│ Filter          │                                                            │
│  [wildcard    ] │                                                            │
│  [Author    ▼ ] │                                                            │
│  [Find][Reset]  │                                                            │
└─────────────────┴──────────────────────────────────────────────────────────┘
```

---

## Features

| Feature | Details |
|---|---|
| **Add articles** | Fill in the left-panel form and click **Add** |
| **Remove articles** | Select one or more rows and click **Remove** |
| **Sort on any column** | Click any column header; click again to reverse |
| **Wildcard filter** | Type a pattern (`Smith*`, `*nano*`, `200?`) and pick a field |
| **Filter any field** | Author, Year, Title, Journal, Volume, or Issue |
| **Clear filter** | Click **Reset** to show all records again |
| **Year validation** | Years later than the current calendar year are rejected |
| **In-place editing** | Double-click any cell; year edits are validated live |
| **Row colour coding** | 🟥 Red = older than 10 years · 🟩 Green = within last 5 years |
| **Dynamic year** | Colours and validation use `QDate::currentDate()` — never hardcoded |

---

## Project Structure

```
JournalDB/
├── JournalDB.pro           # Qt project file
├── main.cpp                # Application entry point
├── mainwindow.h/.cpp       # Main window — layout, slots, wiring
├── articlemodel.h/.cpp     # QStandardItemModel subclass with validation & colouring
├── articlefilterproxy.h/.cpp # QSortFilterProxyModel subclass — wildcard filter + numeric sort
└── README.md
```

### Class Responsibilities

```
main()
  └── MainWindow
        ├── ArticleModel          (data + rules)
        │     • addArticle()      validates year, appends row
        │     • data()            injects background colour per row
        │     • setData()         validates year on in-place edit
        │
        ├── ArticleFilterProxy    (sort + filter)
        │     • setFilter()       stores wildcard + target column
        │     • filterAcceptsRow()wildcardToRegularExpression match
        │     • lessThan()        integer comparison for Year/Vol/Issue
        │
        └── QTableView            (display)
```

---

## Building

### Prerequisites

- **Qt 6** (recommended) or **Qt 5.15+**
- A C++17-capable compiler (GCC 9+, Clang 11+, MSVC 2019+)
- `qmake` or `cmake`

### With qmake

```bash
git clone https://github.com/your-username/JournalDB.git
cd JournalDB
qmake JournalDB.pro
make          # Linux/macOS
# or
nmake         # Windows (MSVC)
# or
mingw32-make  # Windows (MinGW)
```

The binary appears as `JournalDB` (Linux/macOS) or `JournalDB.exe` (Windows).

### With Qt Creator

1. Open **Qt Creator**.
2. **File → Open File or Project** → select `JournalDB.pro`.
3. Configure a kit (Qt 6 or Qt 5.15+).
4. Press **Ctrl+R** to build and run.

---

## Usage Guide

### Adding an Article

1. Fill in all fields in the **Article Details** panel on the left.
   - *Author*, *Title*, and *Journal* must not be empty.
   - *Year* must not be later than the current year (the spin box enforces
     the upper bound; an explicit error dialog appears if validation fails).
2. Click **Add**.
3. The row appears in the table, colour-coded by age.

### Editing In Place

- **Double-click** any cell in the table to edit it directly.
- Editing the **Year** cell enforces the same year ≤ current-year rule.
- The row background updates automatically after a year change.

### Sorting

- Click any **column header** to sort ascending.
- Click again to sort descending.
- Year, Volume, and Issue columns sort **numerically** (not lexicographically).

### Filtering

1. Type a wildcard pattern in the **Filter** text box:
   - `*` matches any sequence of characters.
   - `?` matches exactly one character.
   - Example: `Smith*` finds all authors starting with "Smith".
   - Example: `*review*` finds any title/journal containing "review".
2. Select the **field** to search in from the combo box.
3. Click **Find** (or press **Enter**).
4. Click **Reset** to remove the filter and display all records.

> **Note:** Filtering on *Pages* is intentionally not supported per the spec.

### Removing Articles

1. Click a row to select it (hold **Ctrl** or **Shift** for multiple rows).
2. Click **Remove**.
3. Confirm the deletion in the dialog that appears.

---

## Row Colour Legend

| Colour | Condition |
|---|---|
| 🟥 **Red** (`#FFCCCC`) | Publication year is more than **10 years** before the current year |
| ⬜ **None** | Publication year is between 5 and 10 years ago |
| 🟩 **Green** (`#CCFFCC`) | Publication year is within the **last 5 years** (inclusive) |

The thresholds are evaluated against `QDate::currentDate().year()` at runtime,
so the colours are always accurate regardless of when the application is run.

---

## Design Decisions

### Why `QStandardItemModel` and not a custom model?
The spec explicitly requires `QStandardItemModel`.  We subclass it
(`ArticleModel`) to add validation and colouring while keeping all the
built-in item management.

### Why store integers in `Qt::UserRole`?
`QSortFilterProxyModel::lessThan()` compares `QVariant` values.  Storing
`"9"` and `"10"` as plain strings gives the wrong order.  Storing `9` and
`10` as integers in `Qt::UserRole` lets our `lessThan()` override compare
them correctly.

### Why `QRegularExpression::wildcardToRegularExpression`?
It correctly translates `*` → `.*` and `?` → `.` with proper anchoring and
character-class escaping, handling edge cases that a naïve `replace("*",".*")`
would miss.

### Year validation strategy
Rather than silently clamping an out-of-range year, the application shows a
**`QMessageBox::warning`** and leaves the field unchanged.  This ensures the
user is aware of the constraint and can correct the data intentionally.

---

## Possible Extensions

- Persist the database to a file (JSON / SQLite via `QSqlDatabase`).
- Export filtered results to CSV or BibTeX.
- Add an "import from DOI" feature using the Crossref REST API.
- Multi-column sorting.
- Print / PDF export via `QPrinter`.

---

## License

MIT License — see `LICENSE` for details.

---

## Author

Built as a Qt model-view programming exercise.  
Feel free to fork, extend, and adapt.