#include "comboboxdelegate.h"
#include "wautils.h"
#include "bookform.h"
#include "libraryform.h"
#include "librarymodel.h"
#include <QtWidgets>

LibraryForm::LibraryForm(QWidget *parent) : QWidget(parent)
{
    init(NULL);
}

LibraryForm::LibraryForm(rti_literature *library, QWidget *parent) : QWidget(parent)
{
    init(library);
}


void LibraryForm::setLibrary(rti_literature *library)
{
    libraryModel->setLibrary(library);
    libraryView->resizeColumnsToContents();
}


//
// Private Slots
//
void LibraryForm::selectBooksWithGradeLevel(const QString &gradeLevel)
{
    libraryModel->selectAllBooksInGradeLevel(wa_utils::gradeLevelMap().key(gradeLevel));
}

void LibraryForm::search(const QString &searchTerm)
{
    QRegExp regExp = QRegExp(searchTerm);
    regExp.setCaseSensitivity(Qt::CaseInsensitive);
    proxyModel->setFilterRegExp(regExp);
    proxyModel->setFilterKeyColumn(-1);
}

void LibraryForm::addBook()
{
    BookForm *bookForm = new BookForm;
    connect(bookForm, SIGNAL(bookAdded(rti_book*)), libraryModel, SLOT(addBook(rti_book*)));
    bookForm->show();
}

void LibraryForm::createDictionary()
{
    emit createDictionaryRequested(libraryModel->selectedBooks());
}

void LibraryForm::createFrequencyList()
{
    emit createFrequencyListRequested(libraryModel->selectedBooks());
}


//
// Private Methods
//
void LibraryForm::init(rti_literature *library)
{
    libraryModel = new LibraryModel(library);
    proxyModel = new QSortFilterProxyModel;
    proxyModel->setSourceModel(libraryModel);

    createInterface();
    layoutInterface();
}

void LibraryForm::createInterface()
{
    libraryView = new QTableView;
    libraryView->setModel(proxyModel);
    libraryView->setSortingEnabled(true);
    libraryView->resizeColumnsToContents();
    libraryView->horizontalHeader()->setStretchLastSection(true);
    libraryView->setItemDelegate(new ComboBoxDelegate(4, wa_utils::gradeLevelMap().values()));

    selectAllCheckBox = new QCheckBox(tr("Select all"));
    connect(selectAllCheckBox, SIGNAL(clicked(bool)), libraryModel, SLOT(selectAll(bool)));
    selectGradeLevelLabel = new QLabel(tr("Select all books in grade level"));
    selectGradeLevelComboBox = new QComboBox;
    selectGradeLevelComboBox->addItem("-");
    selectGradeLevelComboBox->addItems(wa_utils::gradeLevelMap().values());
    connect(selectGradeLevelComboBox, SIGNAL(currentIndexChanged(QString)), this, SLOT(selectBooksWithGradeLevel(QString)));
    searchLabel = new QLabel(tr("Search"));
    searchLineEdit = new QLineEdit;
    connect(searchLineEdit, SIGNAL(textChanged(QString)), this, SLOT(search(QString)));

    addBookButton = new QPushButton(tr("Add Book"));
    connect(addBookButton, SIGNAL(clicked()), this, SLOT(addBook()));
    removeBooksButton = new QPushButton(tr("Remove Books"));
    connect(removeBooksButton, SIGNAL(clicked()), libraryModel, SLOT(removeCheckedBooks()));
    createDictionaryButton = new QPushButton(tr("Create Dictionary"));
    connect(createDictionaryButton, SIGNAL(clicked(bool)), this, SLOT(createDictionary()));
    createFrequencyListButton = new QPushButton(tr("Create Frequency List"));
    connect(createFrequencyListButton, SIGNAL(clicked(bool)), this, SLOT(createFrequencyList()));
}

void LibraryForm::layoutInterface()
{
    QHBoxLayout *topLayout = new QHBoxLayout;
    topLayout->addWidget(selectGradeLevelLabel);
    topLayout->addWidget(selectGradeLevelComboBox);
    topLayout->addStretch();
    topLayout->addWidget(selectAllCheckBox);
    topLayout->addStretch();
    topLayout->addWidget(searchLabel);
    topLayout->addWidget(searchLineEdit);

    QHBoxLayout *bottomLayout = new QHBoxLayout;
    bottomLayout->addWidget(addBookButton);
    bottomLayout->addWidget(removeBooksButton);
    bottomLayout->addStretch();
    bottomLayout->addWidget(createDictionaryButton);
    bottomLayout->addWidget(createFrequencyListButton);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addLayout(topLayout);
    mainLayout->addWidget(libraryView);
    mainLayout->addLayout(bottomLayout);
    setLayout(mainLayout);
}
