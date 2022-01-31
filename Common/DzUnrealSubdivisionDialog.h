#pragma once
#include "dzbasicdialog.h"
#include <QtGui/qlineedit.h>
#include <QtGui/qcombobox.h>
#include <QtCore/qsettings.h>
#include "dznode.h"
#include <dzjsonwriter.h>

class QListWidget;
class QListWidgetItem;
class QTreeWidget;
class QTreeWidgetItem;
class QLineEdit;
class QComboBox;
class QGridLayout;


class DzUnrealSubdivisionDialog : public DzBasicDialog {
	Q_OBJECT
	Q_PROPERTY(QObjectList aSubdivisionCombos READ getSubdivisionCombos)
public:
	QObjectList getSubdivisionCombos();

	/** Constructor **/
	DzUnrealSubdivisionDialog(QWidget *parent=nullptr);

	Q_INVOKABLE void PrepareDialog();

	/** Destructor **/
	virtual ~DzUnrealSubdivisionDialog() {}

	Q_INVOKABLE static DzUnrealSubdivisionDialog* Get(QWidget* Parent)
	{
		if (singleton == nullptr)
		{
			singleton = new DzUnrealSubdivisionDialog(Parent);
		}
		singleton->PrepareDialog();
		return singleton;
	}

	QGridLayout* subdivisionItemsGrid;

	Q_INVOKABLE void LockSubdivisionProperties(bool subdivisionEnabled);
	Q_INVOKABLE void WriteSubdivisions(DzJsonWriter& Writer);
	Q_INVOKABLE DzNode* FindObject(DzNode* Node, QString Name);

	Q_INVOKABLE bool setSubdivisionLevelByNode(DzNode* Node, int level);

public slots:
	void HandleSubdivisionLevelChanged(const QString& text);

private:
	void CreateList(DzNode* Node);

	void SavePresetFile(QString filePath);

	QSize minimumSizeHint() const override;

	QString presetsFolder;

	QList<QComboBox*> SubdivisionCombos;

	QMap<QString, int> SubdivisionLevels;

	static DzUnrealSubdivisionDialog* singleton;
};
