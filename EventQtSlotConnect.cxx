#include "EventQtSlotConnect.h"


#include <vtkGenericDataObjectReader.h>

#include <vtkPolyDataMapper.h>
#include <vtkQtTableView.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkSphereSource.h>
#include <vtkSmartPointer.h>
#include <vtkEventQtSlotConnect.h>
#include <vtkInteractorStyleTrackballActor.h>
#include <vtkProperty.h>
#include <vtkLoopSubdivisionFilter.h>

#include <vtkVersion.h>
#include <vtkSmartPointer.h>
#include <vtkActor.h>
#include <vtkSphereSource.h>
#include <vtkRendererCollection.h>
#include <vtkCellArray.h>
#include <vtkInteractorStyleTrackballCamera.h>
#include <vtkObjectFactory.h>
#include <vtkPlaneSource.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkPropPicker.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkPointPicker.h>
#include <vtkVertexGlyphFilter.h>
#include <vtkPolyLine.h>
#include <vtkCellArray.h>
#include <vtkDoubleArray.h>
#include <vtkCellData.h>
#include <vtkCleanPolyData.h>
#include <vtkDataSetMapper.h>
#include <vtkExtractGeometry.h>
#include <vtkAreaPicker.h>

#include <vtkInteractorStyleSwitch.h>
#include <vtkDoubleArray.h>
#include <vtkAppendPolyData.h>

#include <vtkDelaunay3D.h>
#include <vtkDelaunay2D.h>
#include <vtkShrinkFilter.h>
#include <vtkTriangle.h>
#include <vtkPointData.h>
#include <vtkPolygon.h>
#include <vtkPolyDataReader.h>
#include <vtkGenericDataObjectWriter.h>
#include <sstream>
#include <vtkFloatArray.h>
#include <vtkIntArray.h>
#include <vtkStringArray.h>
#include <algorithm>
#include <fstream>
#include <vtkAlgorithmOutput.h>

double pointColor[3];

// Constructor
EventQtSlotConnect::EventQtSlotConnect()
{
  this->setupUi(this);

  this->Connections = vtkSmartPointer<vtkEventQtSlotConnect>::New();

  mouseInteractor = vtkSmartPointer<MouseInteractor>::New();

  //iniTriLabel();

  this->cmrep_progressBar->setMinimum(0);
  this->cmrep_progressBar->setMaximum(100);
  this->cmrep_progressBar->setValue(0);

  this->GridTypeComboBox->addItem("Loop Subdivision");
  
  this->SolverTypeComboBox->addItem("Brute Force");
  this->SolverTypeComboBox->addItem("PDE");

  if(this->GridTypeComboBox->currentIndex() == 0)
	  this->SubLevelComboBox->setEnabled(true);

  if(this->SolverTypeComboBox->currentIndex() == 1)
	  this->RhoLineEdit->setEnabled(true);

  if(this->ConsRadiusCheckBox->isChecked())
	  this->RadiusLineEdit->setEnabled(true);

  createActions();
  createMenus();

  pointColor[0] = 1; pointColor[1] = 0; pointColor[2] = 0;

  //Path browse button
  this->connect(this->pushButtonQvoronoi, SIGNAL(clicked()), this, SLOT(browsePath()));

  //Progress bar
  this->connect(&FutureWatcher, SIGNAL(finished()), this, SLOT(slot_finished()));

  //Excute the cmrepskel
  this->connect(this->cmrepVskel, SIGNAL(clicked()), this, SLOT(executeCmrepVskel()));

  //Mesh interaction
  this->connect(this->checkBoxHideSkel, SIGNAL(stateChanged(int)), this, SLOT(slot_skelStateChange(int)));
  this->connect(this->checkBoxHideMesh, SIGNAL(stateChanged(int)), this, SLOT(slot_meshStateChange(int)));
  
  //Tag modification
  this->connect(this->pushButtonAddTag, SIGNAL(clicked()), this, SLOT(slot_addTag()));
  this->connect(this->comboBoxTagPoint, SIGNAL(activated(int)), this, SLOT(slot_comboxChanged(int)));
  this->connect(this->pushButtonDeleteTag, SIGNAL(clicked()), this, SLOT(slot_delTag()));
  this->connect(this->pushButtonEditTag, SIGNAL(clicked()), this, SLOT(slot_editTag()));

  this->connect(this->pushButtonAddLabel, SIGNAL(clicked()), this, SLOT(slot_addLabel()));
  this->connect(this->pushButtonDeleteLabel, SIGNAL(clicked()), this, SLOT(slot_delLabel()));
  this->connect(this->pushButtonEditLabel, SIGNAL(clicked()), this, SLOT(slot_editLabel()));
  this->connect(this->ChangePtLabelToolButton, SIGNAL(clicked()), this, SLOT(slot_changePtLabel()));

  //Saving option
  this->connect(this->GridTypeComboBox, SIGNAL(activated(int)), this, SLOT(slot_gridTypeChanged(int)));
  this->connect(this->SolverTypeComboBox, SIGNAL(activated(int)), this, SLOT(slot_solverTypeChanged(int)));
  this->connect(this->ConsRadiusCheckBox, SIGNAL(stateChanged(int)), this, SLOT(slot_consRadiusCheck(int)));

  //Toggle triangle labels button
  this->connect(this->pushButtonToggleTri, SIGNAL(clicked()), this, SLOT(slot_toggleTriLabel()));

  //Change tag radius/size
  this->connect(this->TagSizeSlider, SIGNAL(valueChanged(int)), this, SLOT(slot_tagSizeSlider(int)));

  //operation signal
  this->connect(this->AddPointToolButton, SIGNAL(clicked()), this, SLOT(slot_addPoint()));
  this->connect(this->DelPointToolButton, SIGNAL(clicked()), this, SLOT(slot_deletePoint()));
  this->connect(this->CreateTriToolButton, SIGNAL(clicked()), this, SLOT(slot_createTri()));
  this->connect(this->DelTriToolButton, SIGNAL(clicked()), this, SLOT(slot_deleteTri()));
  this->connect(this->FlipNormalToolButton, SIGNAL(clicked()), this, SLOT(slot_flipNormal()));
  this->connect(this->ViewToolButton, SIGNAL(clicked()), this, SLOT(slot_view()));
  this->connect(this->ChangeTriLabelButton, SIGNAL(clicked()), this, SLOT(slot_changeTriLabel()));
  this->connect(this->MovePtToolButton, SIGNAL(clicked()), this, SLOT(slot_movePoint()));


  this->connect(mouseInteractor, SIGNAL(skelStateChanged(int)), this, SLOT(slot_skelStateChange(int)));
  this->connect(mouseInteractor, SIGNAL(meshStateChanged(int)), this, SLOT(slot_meshStateChange(int)));

  //update label
  this->connect(mouseInteractor, SIGNAL(operationChanged(int)), this, SLOT(slot_updateOperation(int)));

  //update progress bar
  this->connect(&v, SIGNAL(progressChanged()), this, SLOT(slot_updateProgressBar()));
  progressSignalCount = 0;

  //transparent Slider
  this->connect(this->SkelTransparentSlider, SIGNAL(valueChanged(int)), this, SLOT(slot_skelTransparentChanged(int)));
  this->connect(this->MeshTransparentSlider, SIGNAL(valueChanged(int)), this, SLOT(slot_meshTransparentChanged(int)));

  //triangle label
  this->connect(this->TriLabelComboBox, SIGNAL(activated(int)), this, SLOT(slot_trilabelChanged(int)));

  //background color
  this->connect(this->pushButtonBckgndColor, SIGNAL(clicked()), this, SLOT(slot_setColor()));

  //inflate option
  this->connect(this->InflateCheckBox, SIGNAL(stateChanged(int)), this, SLOT(slot_inflate(int)));

  //preview subdivision option
  this->connect(this->previewSubdivideCheckBox, SIGNAL(stateChanged(int)), this, SLOT(slot_subdivisionPreview(int)));

  //preview button
  this->connect(this->previewPushButton, SIGNAL(clicked()), this, SLOT(slot_preview()));

  //set some default value
  this->eParameter->setValue(2);
  this->pParameter->setValue(1.2);
  this->cParameter->setValue(0);
  this->tParameter->setValue(1e-6);
  colorBckgnd.setRed(0);
  colorBckgnd.setGreen(0);
  colorBckgnd.setBlue(1);

  std::string setname = "settings.ini";

  if (!(bool)std::ifstream(setname.c_str())) {
	  std::ofstream setfile("settings.ini");
	  setfile.close();
  }

  settingsFile = QApplication::applicationDirPath() + "/settings.ini";
  loadSettings(); 

  vtkSmartPointer<vtkRenderer> renderer = 
	  vtkSmartPointer<vtkRenderer>::New();
  this->qvtkWidget->GetRenderWindow()->AddRenderer(renderer);
  this->qvtkWidget->update();

  timer = new QTimer;
  connect(timer, SIGNAL(timeout()), this, SLOT(autoSave()));
  
};

EventQtSlotConnect::~EventQtSlotConnect()
{
	saveSettings();
}

void EventQtSlotConnect::autoSave() { 
	std::string fileTemp = VTKfilename.substr(0, VTKfilename.length() - 4).append("Temp.vtk");
	std::cout << "Automatically saved" << std::endl;
	saveVTKFile(QString::fromStdString(fileTemp));
}
//Call the add tag dialog
void EventQtSlotConnect::slot_addTag(){

	AddTagDialog addDialog;
	addDialog.show();

	if(addDialog.exec()){
		QString tagText = addDialog.lineEdit->text();
		
		TagInfo ti;
		ti.tagName = tagText.toStdString();
		ti.qc = addDialog.color;
		int r, g, b;
		r = addDialog.color.red();
		g = addDialog.color.green();
		b = addDialog.color.blue();
		//ti.tagColor[0] = ((double) std::rand() / (RAND_MAX)); ti.tagColor[1] = ((double) std::rand() / (RAND_MAX)); ti.tagColor[2] = ((double) std::rand() / (RAND_MAX));
		ti.tagColor[0] = r; ti.tagColor[1] = g; ti.tagColor[2] = b;
		if (addDialog.branchButton->isChecked()) 
			ti.tagType = 1;
		else if(addDialog.freeEdgeButton->isChecked())
			ti.tagType = 2;
		else if(addDialog.interiorButton->isChecked())
			ti.tagType = 3;
		else
			ti.tagType = 4;
		ti.tagIndex = addDialog.tagIndex;

		//Store the tag information
		Global::vectorTagInfo.push_back(ti);

		QPixmap pix(22,22);
		QString displayText = QString::number(ti.tagIndex) + " " + tagText;
		pix.fill(addDialog.color);
		this->comboBoxTagPoint->addItem(pix, displayText);
	}	
}

void EventQtSlotConnect::slot_delTag()
{
	if(Global::vectorTagInfo.size() != 0){
		int curIndex = this->comboBoxTagPoint->currentIndex();
		QString tagName = this->comboBoxTagPoint->currentText();
		for(int i = 0; i < Global::vectorTagPoints.size(); i++)
		{
			if(curIndex == Global::vectorTagPoints[i].comboBoxIndex)
			{
				QMessageBox messageBox;
				messageBox.critical(0,"Error","You need to delete the remaining point(s) before deleting this tag: " + tagName);
				return;
			}
		}
		switch( QMessageBox::information( this, "Delete Tag",
			"Are you sure to delete this tag (" + tagName + ")? ",
			"Yes", "Cancel",
			0, 1 ) ) {
		case 0:
			Global::vectorTagInfo.erase(Global::vectorTagInfo.begin() + curIndex);
			this->comboBoxTagPoint->removeItem(curIndex);
			//new index
			Global::selectedTag = this->comboBoxTagPoint->currentIndex();
			break;
		case 1:
		default:
			break;	
		}	
	}
	else
	{
		QMessageBox warning;
		warning.critical(0, "Warning", "No existing tag");
		return;
	}
}

void EventQtSlotConnect::slot_editTag()
{
	if(Global::vectorTagInfo.size()>0)
	{
		AddTagDialog addDialog;

		TagInfo tio = Global::vectorTagInfo[comboBoxTagPoint->currentIndex()];

		addDialog.lineEdit->setText(QString::fromStdString(tio.tagName));
		addDialog.color = tio.qc;
		addDialog.colorLabel->setPalette(QPalette(tio.qc));
		addDialog.colorLabel->setAutoFillBackground(true);
		if(tio.tagType == 1)
			addDialog.branchButton->setChecked(true);
		else if(tio.tagType == 2)
			addDialog.freeEdgeButton->setChecked(true);
		else if(tio.tagType == 3)
			addDialog.interiorButton->setChecked(true);
		else
			addDialog.otherButton->setChecked(true);

		addDialog.indexBox->setCurrentIndex(tio.tagIndex-1);
		addDialog.tagIndex = tio.tagIndex;

		addDialog.show();
	
		if(addDialog.exec())
		{
			QString tagText = addDialog.lineEdit->text();

			TagInfo ti;
			ti.tagName = tagText.toStdString();
			ti.qc = addDialog.color;
			int r, g, b;
			r = addDialog.color.red();
			g = addDialog.color.green();
			b = addDialog.color.blue();
			ti.tagColor[0] = r; ti.tagColor[1] = g; ti.tagColor[2] = b;
			if(addDialog.branchButton->isChecked())
				ti.tagType = 1;
			else if(addDialog.freeEdgeButton->isChecked())
				ti.tagType = 2;
			else if(addDialog.interiorButton->isChecked())
				ti.tagType = 3;
			else
				ti.tagType = 4;
			ti.tagIndex = addDialog.tagIndex;

			Global::vectorTagInfo[this->comboBoxTagPoint->currentIndex()] = ti;

			//update the tag point on skeleton
			for(int i = 0; i < Global::vectorTagPoints.size(); i++)
			{
				if(Global::vectorTagPoints[i].comboBoxIndex == this->comboBoxTagPoint->currentIndex())
				{
					Global::vectorTagPoints[i].typeIndex = ti.tagType;
					Global::vectorTagPoints[i].typeName = ti.tagName;
					Global::vectorTagPoints[i].actor->GetProperty()->SetColor(ti.tagColor[0] / 255.0, ti.tagColor[1] / 255.0, ti.tagColor[2] / 255.0);
				}
			}

			//update combobox
			QPixmap pix(22,22);
			QString displayText = QString::number(ti.tagIndex) + " " + tagText;
			pix.fill(addDialog.color);
			int tempIndex = this->comboBoxTagPoint->currentIndex();
			this->comboBoxTagPoint->removeItem(tempIndex);
			this->comboBoxTagPoint->insertItem(tempIndex, pix, displayText);
		}
	}
}

void EventQtSlotConnect::slot_addLabel()
{
	AddLabelDialog addLabel;
    addLabel.show();

    if(addLabel.exec()){

		LabelTriangle lt;

		QPixmap pix(22, 22);
		QColor qc = addLabel.color;
		int index = addLabel.index->text().toInt();
		QString labelText = addLabel.nameEdit->text();

		lt.labelName = addLabel.nameEdit->text().toStdString();
		lt.labelColor = qc;
		Global::vectorLabelInfo.push_back(lt);

		triLabelColors.push_back(qc);
		hideTriLabel.push_back(0);
		mouseInteractor->triLabelColors.push_back(qc);

		if (index-1 == 0) {
			Global::triCol[0] = qc.red() / 255.0;
			Global::triCol[1] = qc.green() / 255.0;
			Global::triCol[2] = qc.blue() / 255.0;
			mouseInteractor->currentTriIndex = index-1;
		}
		pix.fill(qc);
		this->TriLabelComboBox->addItem(pix, labelText);
    }
}

void EventQtSlotConnect::slot_delLabel()
{
	std::cout << Global::vectorLabelInfo.size() << std::endl;
	if (Global::vectorLabelInfo.size() != 0) {
		int curIndex = this->TriLabelComboBox->currentIndex();
		QString labelName = this->TriLabelComboBox->currentText();
		for (int i = 0; i < Global::vectorTagTriangles.size(); i++) {
			if (curIndex == Global::vectorTagTriangles[i].index)
			{
				QMessageBox messageBox;
				messageBox.critical(0, "Error", "You need to delete the remaining triangle(s) before deleting this label: " + labelName);
				return;
			}
		}
		
		switch (QMessageBox::information(this, "Delete Label",
			"Are you sure to delete this tag ("+ labelName+")? ",
			"Yes", "Cancel",
			0, 1)) {
		case 0:
		{
			Global::vectorLabelInfo.erase(Global::vectorLabelInfo.begin() + curIndex);
			this->TriLabelComboBox->removeItem(curIndex);
			triLabelColors.erase(triLabelColors.begin() + curIndex);
			hideTriLabel.erase(hideTriLabel.begin() + curIndex);
			mouseInteractor->triLabelColors.erase(mouseInteractor->triLabelColors.begin() + curIndex);
			int newIndex = this->TriLabelComboBox->currentIndex();
			mouseInteractor->currentTriIndex = newIndex;
			break;
		}
		case 1:
		default:
			break;
		}
	}
	else
	{
		QMessageBox warning;
		warning.critical(0, "Warning", "No existing label");
		return;
	}
}

void EventQtSlotConnect::slot_editLabel()
{
	if (Global::vectorLabelInfo.size() > 0)
	{
		AddLabelDialog addLabel;
		int curIndex = TriLabelComboBox->currentIndex();
		LabelTriangle lto = Global::vectorLabelInfo[curIndex];

		addLabel.nameEdit->setText(QString::fromStdString(lto.labelName));
		addLabel.color = lto.labelColor;
		addLabel.colorLabel->setPalette(QPalette(lto.labelColor));
		addLabel.colorLabel->setAutoFillBackground(true);
		addLabel.index->setText(QString::number(curIndex + 1));

		addLabel.show();
		if (addLabel.exec())
		{
			QString labelText = addLabel.nameEdit->text();
			QColor labelColor = addLabel.color;
			
			LabelTriangle lt;
			lt.labelName = labelText.toStdString();
			lt.labelColor = labelColor;

			Global::vectorLabelInfo[curIndex] = lt;

			triLabelColors[curIndex] = labelColor;
			mouseInteractor->triLabelColors[curIndex] = labelColor;

			for (int i = 0; i < Global::vectorTagTriangles.size(); i++) 
			{
				if (Global::vectorTagTriangles[i].index == curIndex)
				{
					Global::vectorTagTriangles[i].triActor->GetProperty()->SetColor(
						labelColor.red() / 255.0,
						labelColor.green() / 255.0,
						labelColor.blue() / 255.0);
				}
			}

			QPixmap pix(22, 22);
			pix.fill(labelColor);
			this->TriLabelComboBox->removeItem(curIndex);
			this->TriLabelComboBox->insertItem(curIndex, pix, labelText);
		}
	}
}

void EventQtSlotConnect::slot_changePtLabel()
{
    mouseInteractor->operationFlag = EDITTAGPT;
    this->OperationModelLabel->setText("Change Point Label");
    setToolButton(EDITTAGPT);
    mouseInteractor->preKey = "";
    mouseInteractor->reset();

}

void EventQtSlotConnect::slot_finished()
{
	readVTK(VTKfilename);
	this->cmrep_progressBar->setMaximum(100);
	this->cmrep_progressBar->setMinimum(0);
	this->cmrep_progressBar->setValue(0);
	progressSignalCount = 0;
	//this->cmrep_progressBar->hide();
}

void EventQtSlotConnect::browsePath()
{
	QString directory =
		QDir::toNativeSeparators(QFileDialog::getOpenFileName(this, tr("Qvoronoi path"), QDir::currentPath()));

	if (!directory.isEmpty()) {
		this->pathQvoronoi->setText(directory);
	}
}

void EventQtSlotConnect::slot_open(){
	QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"), QDir::currentPath());
	if (!fileName.isEmpty()) {
		VTKfilename = fileName.toStdString();
		readVTK(VTKfilename);
		timer->start(60000);
	}
}

void EventQtSlotConnect::slot_save(){

	QFileDialog::Options options;
	//if (!native->isChecked())
	//	options |= QFileDialog::DontUseNativeDialog;
	QString selectedFilter;
	QString fileName = QFileDialog::getSaveFileName(this,
		tr("Save File"),
		tr(""),
		tr("VTK Files (*.vtk)"),
		&selectedFilter,
		options);	

	if (!fileName.isEmpty()){	
		saveVTKFile(fileName);
		saveParaViewFile(fileName);
		saveCmrepFile(fileName);	
		exportSubdivideMesh(fileName);
		if (this->InflateCheckBox->isChecked()) {
			double radius = this->radiusSpinBox->value();
			InflateMedialModelWithBranches(fileName, radius);
		}
		
	}
}

void EventQtSlotConnect::slot_import()
{
	importNiftiiWindow inw;
	inw.show();
	if (inw.exec())
	{
		std::vector<std::string> filenames;
		filenames.push_back(inw.inputEdit->text().toStdString());
		filenames.push_back(inw.outputEdit->text().toStdString());

		std::cout << "Input filename: " << filenames[0] << "\nOutput filename: " << filenames[1] << std::endl;
		if (inw.smoothCheck->isChecked()) 
		{
			std::string sigma = inw.sigmaEdit->text().toStdString();

			std::vector<std::string> th1Param;

			std::string u11Temp = inw.u11->text().toStdString();
			std::transform(u11Temp.begin(), u11Temp.end(), u11Temp.begin(), ::tolower);
			std::string u21Temp = inw.u21->text().toStdString();
			std::transform(u21Temp.begin(), u21Temp.end(), u21Temp.begin(), ::tolower);

			th1Param.push_back(u11Temp);
			th1Param.push_back(u21Temp);
			th1Param.push_back("1"); 
			th1Param.push_back("0"); 

			std::vector<std::string> th2Param;

			std::string u12Temp = inw.u12->text().toStdString();
			std::transform(u12Temp.begin(), u12Temp.end(), u12Temp.begin(), ::tolower);
			std::string u22Temp = inw.u22->text().toStdString();
			std::transform(u22Temp.begin(), u22Temp.end(), u22Temp.begin(), ::tolower);

			th2Param.push_back(u12Temp);
			th2Param.push_back(u22Temp);
			th2Param.push_back("1"); 
			th2Param.push_back("0"); 

			std::cout << "\nSigma value: " << sigma << " vox" << std::endl;
			std::cout << "\nPre-thresholding parameters: \nu1: " << th1Param[0] << " u2: " << th1Param[1] << " v1: " << th1Param[2] << " v2: " << th1Param[3] << std::endl;
			std::cout << "\nPost-thresholding parameters: \nu1: " << th2Param[0] << " u2: " << th2Param[1] << " v1: " << th2Param[2] << " v2: " << th2Param[3] << std::endl;

			importNIFTI(filenames, true, sigma, th1Param, th2Param);
		}
		else
			importNIFTI(filenames, false);
	}
}

void EventQtSlotConnect::slot_preview()
{
	std::vector<double> radiusData;
	vtkSmartPointer<vtkPolyData> previewMesh =
		vtkSmartPointer<vtkPolyData>::New();
	int numberOfSubdivisions;
	if (this->previewSubdivideCheckBox->isChecked()) {
		numberOfSubdivisions = this->previewSubdivideComboBox->currentIndex() + 1;
	}
	else {
		numberOfSubdivisions = 0;
	}

	if (Global::vectorTagTriangles.size() > 0) {

		vtkSmartPointer<vtkAppendPolyData> appendFilter =
			vtkSmartPointer<vtkAppendPolyData>::New();

		for (int i = 0; i < Global::vectorTagTriangles.size(); i++)
		{
			vtkSmartPointer<vtkActorCollection> actorCollection =
				vtkSmartPointer<vtkActorCollection>::New();
			Global::vectorTagTriangles[i].triActor->GetActors(actorCollection);
			vtkPolyData* polyData = vtkPolyData::SafeDownCast(actorCollection->GetLastActor()->GetMapper()->GetInput());
			appendFilter->AddInputData(polyData);
		}

		vtkSmartPointer<vtkCleanPolyData> cleanPoly =
			vtkSmartPointer<vtkCleanPolyData>::New();

		cleanPoly->SetInputConnection(appendFilter->GetOutputPort());
		cleanPoly->Update();

		if (numberOfSubdivisions > 0) {
			vtkSmartPointer<vtkLoopSubdivisionFilter> subdivisionFilter =
				vtkSmartPointer<vtkLoopSubdivisionFilter>::New();

			subdivisionFilter->SetNumberOfSubdivisions(numberOfSubdivisions);
			subdivisionFilter->SetInputData(cleanPoly->GetOutput());
			subdivisionFilter->Update();

			vtkSmartPointer<vtkActorCollection> actors = mouseInteractor->GetDefaultRenderer()->GetActors();
			vtkSmartPointer<vtkActor> actor0 = static_cast<vtkActor *>(actors->GetItemAsObject(0));
			vtkSmartPointer<vtkDataSet> vtkdata = actor0->GetMapper()->GetInputAsDataSet();
			vtkDoubleArray* radiusArray = (vtkDoubleArray*)vtkdata->GetPointData()->GetArray("Radius");
			vtkSmartPointer<vtkPolyData> outFilter = subdivisionFilter->GetOutput();

			for (int i = 0; i < subdivisionFilter->GetOutput()->GetNumberOfPoints(); i++)
			{
				double pos[3];
				outFilter->GetPoint(i, pos);

				double minDistance = DBL_MAX;
				double pointRadius = 0.0;

				for (vtkIdType j = 0; j < vtkdata->GetNumberOfPoints(); j++) {
					double p[3];
					vtkdata->GetPoint(j, p);

					double dist = std::sqrt(std::pow(pos[0] - p[0], 2) + std::pow(pos[1] - p[1], 2) + std::pow(pos[2] - p[2], 2));
					//Find the closet vertex
					if (dist < minDistance) {
						minDistance = dist;
						pointRadius = radiusArray->GetValue(j);
					}
				}
				radiusData.push_back(pointRadius);
			}

			previewMesh = subdivisionFilter->GetOutput();
		}
		else {
			for (int i = 0; i < cleanPoly->GetOutput()->GetNumberOfPoints(); i++) {
				for (int j = 0; j < Global::vectorTagPoints.size(); j++)
				{
					if (Global::vectorTagPoints[j].pos[0] == cleanPoly->GetOutput()->GetPoint(i)[0] &&
						Global::vectorTagPoints[j].pos[1] == cleanPoly->GetOutput()->GetPoint(i)[1] &&
						Global::vectorTagPoints[j].pos[2] == cleanPoly->GetOutput()->GetPoint(i)[2])
					{
						radiusData.push_back(Global::vectorTagPoints[j].radius);
					}
				}
			}

			previewMesh = cleanPoly->GetOutput();
		}
	}

	PreviewWindow *pw = new PreviewWindow(VTKfilename, radiusData, previewMesh);
	pw->show(); 
}

void EventQtSlotConnect::slot_inflate(int state) {
	if (state == Qt::Unchecked) {
		this->InflateCheckBox->setChecked(false);
		this->radiusSpinBox->setDisabled(true);
	}
	else {
		this->InflateCheckBox->setChecked(true);
		this->radiusSpinBox->setDisabled(false);
	}
}

void EventQtSlotConnect::slot_subdivisionPreview(int state)
{
	if (state == Qt::Unchecked) {
		this->previewSubdivideCheckBox->setChecked(false);
		this->previewSubdivideComboBox->setDisabled(true);
	}
	else {
		this->previewSubdivideCheckBox->setChecked(true);
		this->previewSubdivideComboBox->setDisabled(false);
	}
}

void EventQtSlotConnect::slot_skelStateChange(int state){
	
	vtkRendererCollection* rendercollection = this->qvtkWidget->GetRenderWindow()->GetRenderers();
	vtkRenderer* render = rendercollection->GetFirstRenderer();
	vtkActorCollection* actorcollection = render->GetActors();
	actorcollection->InitTraversal();
	vtkActor* actor = actorcollection->GetNextActor();
	if(state == Qt::Unchecked){
		actor->VisibilityOn();
		this->checkBoxHideSkel->setChecked(false);
		mouseInteractor->skelState = SHOW;
	}
	else{
		actor->VisibilityOff();
		this->checkBoxHideSkel->setChecked(true);
		mouseInteractor->skelState = HIDE;
	}
	//render->ResetCamera();
	this->qvtkWidget->GetRenderWindow()->Render();
}

void EventQtSlotConnect::slot_meshStateChange(int state){
	if(state == Qt::Unchecked)
	{
		this->checkBoxHideMesh->setChecked(false);
		mouseInteractor->meshState = SHOW;
		for(int i = 0; i < Global::vectorTagTriangles.size(); i++)
		{
			Global::vectorTagTriangles[i].triActor->VisibilityOn();
		}
		for (int i = 0; i < hideTriLabel.size(); i++) {
			hideTriLabel[i] = 0;
		}
	}
	else
	{
		this->checkBoxHideMesh->setChecked(true);
		mouseInteractor->meshState = HIDE;
		for(int i = 0; i < Global::vectorTagTriangles.size(); i++)
		{
			Global::vectorTagTriangles[i].triActor->VisibilityOff();
		}
		for (int i = 0; i < hideTriLabel.size(); i++) {
			hideTriLabel[i] = 1;
		}
	}
	this->qvtkWidget->GetRenderWindow()->Render();
}

void EventQtSlotConnect::slot_comboxChanged(int state)
{
    Global::selectedTag = this->comboBoxTagPoint->currentIndex();
}

void EventQtSlotConnect::slot_gridTypeChanged(int state)
{
	if(state == 0)
		this->SubLevelComboBox->setEnabled(true);
	else
		this->SubLevelComboBox->setEnabled(false);
}

void EventQtSlotConnect::slot_solverTypeChanged(int state)
{
	if(state == 1)
		this->RhoLineEdit->setEnabled(true);
	else
		this->RhoLineEdit->setEnabled(false);
}

void EventQtSlotConnect::slot_consRadiusCheck(int state)
{
	if(state == Qt::Checked)
		this->RadiusLineEdit->setEnabled(true);
	else
		this->RadiusLineEdit->setEnabled(false);
}

void EventQtSlotConnect::slot_toggleTriLabel() 
{
    ToggleTriLabel toggleTriLabel(hideTriLabel);
    toggleTriLabel.show();

    if (toggleTriLabel.exec()) {
		for (int i = 0; i < Global::vectorLabelInfo.size(); i++) {
			if (toggleTriLabel.listCheckBox[i]->isChecked()) {
				hideTriLabel[i] = 1;
				this->TriLabelComboBox->setItemText(i, QString::fromStdString(Global::vectorLabelInfo[i].labelName + " (hidden)"));
			}
			else {
				hideTriLabel[i] = 0;
				this->TriLabelComboBox->setItemText(i, QString::fromStdString(Global::vectorLabelInfo[i].labelName));
			}
		}
	}

	for (int i = 0; i < Global::vectorTagTriangles.size(); i++) {
		int triIndex = Global::vectorTagTriangles[i].index;
		if (hideTriLabel[triIndex] == 1) {
			Global::vectorTagTriangles[i].triActor->VisibilityOff();
		}
		else {
			Global::vectorTagTriangles[i].triActor->VisibilityOn();
		}
	}
	this->qvtkWidget->GetRenderWindow()->Render();
}

void EventQtSlotConnect::slot_tagSizeSlider(int value)
{	
	double tsize = value / 10.0;
	Global::tagRadius = tsize;
	for(int i = 0; i < Global::vectorTagPoints.size(); i++)
	{
		vtkActor* tagPt = Global::vectorTagPoints[i].actor;
		vtkSmartPointer<vtkAlgorithm> algorithm =
			tagPt->GetMapper()->GetInputConnection(0, 0)->GetProducer();
		vtkSmartPointer<vtkSphereSource> srcReference =
			vtkSphereSource::SafeDownCast(algorithm);
		srcReference->SetRadius(tsize);
	}
	this->qvtkWidget->update();
}

//Enable and disable tool button
void EventQtSlotConnect::setToolButton(int flag)
{

	this->AddPointToolButton->setEnabled(true);
	this->DelPointToolButton->setEnabled(true);
	this->CreateTriToolButton->setEnabled(true);
	this->DelTriToolButton->setEnabled(true);
	this->ViewToolButton->setEnabled(true);
	this->FlipNormalToolButton->setEnabled(true);
	this->ChangeTriLabelButton->setEnabled(true);
	this->MovePtToolButton->setEnabled(true);
    this->ChangePtLabelToolButton->setEnabled(true);

	if(flag == ADDPOINT)
		this->AddPointToolButton->setEnabled(false);
	else if(flag == DELETEPOINT)
		this->DelPointToolButton->setEnabled(false);
	else if(flag == CREATETRI)
		this->CreateTriToolButton->setEnabled(false);
	else if(flag == DELETETRI)
		this->DelTriToolButton->setEnabled(false);
	else if(flag == FLIPNORMAL)
		this->FlipNormalToolButton->setEnabled(false);
	else if(flag == VIEW)
		this->ViewToolButton->setEnabled(false);
	else if(flag == CHANGETRILABEL)
		this->ChangeTriLabelButton->setEnabled(false);
	else if(flag == MOVEPT)
		this->MovePtToolButton->setEnabled(false);
    else if(flag == EDITTAGPT)
        this->ChangePtLabelToolButton->setEnabled(false);

	this->qvtkWidget->update();
}

void EventQtSlotConnect::slot_addPoint()
{
	if (Global::vectorTagInfo.size() == 0)
	{
		QMessageBox warning;
		warning.critical(0, "Warning", "You need to create a point label before creating a point");
		return;
	}
	else
	{
		mouseInteractor->operationFlag = ADDPOINT;
		this->OperationModelLabel->setText("Add Point");
		setToolButton(ADDPOINT);
		mouseInteractor->preKey = "";
		mouseInteractor->reset();
	}
}

void EventQtSlotConnect::slot_deletePoint()
{
	mouseInteractor->operationFlag = DELETEPOINT;
	this->OperationModelLabel->setText("Delete Point");
	setToolButton(DELETEPOINT);
	mouseInteractor->preKey = "";
	mouseInteractor->reset();
}

void EventQtSlotConnect::slot_createTri()
{
	if (Global::vectorLabelInfo.size() == 0)
	{
		QMessageBox warning;
		warning.critical(0, "Warning", "You need to create a triangle label before creating a triangle");
		return;
	}
	else
	{
		mouseInteractor->operationFlag = CREATETRI;
		this->OperationModelLabel->setText("Add Triangle");
		setToolButton(CREATETRI);
		mouseInteractor->preKey = "";
		mouseInteractor->reset();
	}
}

void EventQtSlotConnect::slot_deleteTri()
{
	mouseInteractor->operationFlag = DELETETRI;
	this->OperationModelLabel->setText("Delete Triangle");
	setToolButton(DELETETRI);
	mouseInteractor->preKey = "";
	mouseInteractor->reset();
}

void EventQtSlotConnect::slot_flipNormal()
{
	mouseInteractor->operationFlag = FLIPNORMAL;	
	this->OperationModelLabel->setText("Flip Normal");
	setToolButton(FLIPNORMAL);
	mouseInteractor->preKey = "";
	mouseInteractor->reset();
}

void EventQtSlotConnect::slot_view()
{
	mouseInteractor->operationFlag = VIEW;
	this->OperationModelLabel->setText("View");
	setToolButton(VIEW);
	mouseInteractor->preKey = "";
	mouseInteractor->reset();
}

void EventQtSlotConnect::slot_changeTriLabel()
{
	mouseInteractor->operationFlag = CHANGETRILABEL;
	this->OperationModelLabel->setText("Change Triangle Label");
	setToolButton(CHANGETRILABEL);
	mouseInteractor->preKey = "";
	mouseInteractor->reset();
}

void EventQtSlotConnect::slot_movePoint()
{
	mouseInteractor->operationFlag = MOVEPT;
	this->OperationModelLabel->setText("Move Point");
	setToolButton(MOVEPT);
	mouseInteractor->preKey = "";
	mouseInteractor->reset();
}

void EventQtSlotConnect::slot_updateOperation(int state)
{
	if(state == ADDPOINT)
		this->OperationModelLabel->setText("Add Point");
	else if(state == DELETEPOINT)
		this->OperationModelLabel->setText("Delete Point");
	else if(state == CREATETRI)
		this->OperationModelLabel->setText("Add Triangle");
	else if(state == DELETETRI)
		this->OperationModelLabel->setText("Delete Triangle");
	else if(state == FLIPNORMAL)
		this->OperationModelLabel->setText("Flip Normal");
	else if(state == VIEW)
		this->OperationModelLabel->setText("View");
	else if(state == CHANGETRILABEL)
		this->OperationModelLabel->setText("Change Triangle Label");
	else if(state == MOVEPT)
		this->OperationModelLabel->setText("Move Point");
	
	setToolButton(state);
}

void EventQtSlotConnect::slot_updateProgressBar()
{
	this->cmrep_progressBar->setValue(++progressSignalCount);
}

void EventQtSlotConnect::slot_skelTransparentChanged(int value)
{
	vtkRendererCollection* rendercollection = this->qvtkWidget->GetRenderWindow()->GetRenderers();
	vtkRenderer* render = rendercollection->GetFirstRenderer();
	vtkActorCollection* actorcollection = render->GetActors();
	actorcollection->InitTraversal();
	vtkActor* actor = actorcollection->GetNextActor();
	if(actor != NULL){
		double trans = value / 100.0;
		actor->GetProperty()->SetOpacity(trans);
		this->qvtkWidget->update();
	}
	this->qvtkWidget->GetRenderWindow()->Render();
}

void EventQtSlotConnect::slot_meshTransparentChanged(int value)
{
	double trans = value / 100.0;
	for (int i = 0; i < Global::vectorTagTriangles.size(); i++)
	{
		Global::vectorTagTriangles[i].triActor->GetProperty()->SetOpacity(trans);
		this->qvtkWidget->update();
	}
	this->qvtkWidget->GetRenderWindow()->Render();
}

void EventQtSlotConnect::slot_trilabelChanged(int index)	
{
	int curIndex = this->TriLabelComboBox->currentIndex();
	Global::triCol[0] = triLabelColors[curIndex].red() / 255.0;
	Global::triCol[1] = triLabelColors[curIndex].green() / 255.0;
	Global::triCol[2] = triLabelColors[curIndex].blue() / 255.0;
	mouseInteractor->currentTriIndex = curIndex;
}

void EventQtSlotConnect::slot_setColor()
{
    colorBckgnd = QColorDialog::getColor(Qt::green, this, "Select Color", QColorDialog::DontUseNativeDialog);
    vtkSmartPointer<vtkRenderer> renderer =
        vtkSmartPointer<vtkRenderer>::New();
    if (colorBckgnd.isValid()) {
        if(this->qvtkWidget->GetRenderWindow()->GetRenderers()->GetFirstRenderer() != NULL)
            this->qvtkWidget->GetRenderWindow()->GetRenderers()->GetFirstRenderer()->SetBackground(colorBckgnd.red()/255.0,colorBckgnd.green()/255.0,colorBckgnd.blue()/255.0);
            this->qvtkWidget->update();
    }
}

void EventQtSlotConnect::executeCmrepVskel()
{
	std::vector <char *> parameters;
	parameters.push_back("cmrep_vskel");
	
	std::string pathText = this->pathQvoronoi->text().toStdString();
	if(!pathText.empty()){
		parameters.push_back("-Q");
		char *temp = new char[256];
		strcpy(temp, pathText.c_str());
		parameters.push_back(temp);
	}
	
	int evalue = this->eParameter->value();
	if(evalue != 0){
		parameters.push_back("-e");
		char *temp = new char[256];
		std::stringstream ss;
		ss << evalue;
		std::string tempS = ss.str();
		strcpy(temp, tempS.c_str());
		parameters.push_back(temp);
	}

	double pvalue = this->pParameter->value();
	if(pvalue != 0.0){
		parameters.push_back("-p");
		char *temp = new char[256];
		std::stringstream ss;
		ss << pvalue;
		std::string tempS = ss.str();
		strcpy(temp, tempS.c_str());
		parameters.push_back(temp);
	}

	int cvalue = this->cParameter->value();
	if(cvalue != 0){
		parameters.push_back("-c");
		char temp[256];
		sprintf(temp, "%d", cvalue);
		parameters.push_back(temp);
	}

	double tvalue = this->tParameter->value();
	if(tvalue != 0.0){
		parameters.push_back("-t");
		char temp[256];
		sprintf(temp, "%g", tvalue);
		parameters.push_back(temp);
	}

	QString qtextQ = this->qParameter->text();
	std::string qtext = qtextQ.toStdString();
	if(!qtext.empty()){
		parameters.push_back("-q");
		char *temp = new char;
		strcpy(temp, qtext.c_str());
		parameters.push_back(temp);
	}
	
	char *command[3];
	command[1] = new char[VTKfilename.length() + 1];
	strcpy(command[1], VTKfilename.c_str());
	parameters.push_back(command[1]);

	std::string outputNameSkel =  VTKfilename;
	outputNameSkel = outputNameSkel.substr(0, outputNameSkel.length() - 4) + "_Skel.vtk";
	command[2] = new char [outputNameSkel.length() + 1];
	strcpy(command[2], outputNameSkel.c_str());
	parameters.push_back(command[2]);
	VTKfilename = outputNameSkel;

	QFuture<void> future = QtConcurrent::run(&this->v, &VoronoiSkeletonTool::execute, parameters.size(), parameters);
	this->FutureWatcher.setFuture(future);	
}

void EventQtSlotConnect::createActions()
{
	openAct = new QAction(tr("&Open..."), this);
	openAct->setShortcut(tr("Ctrl+O"));
	connect(openAct, SIGNAL(triggered()), this, SLOT(slot_open()));

	saveAct = new QAction(tr("&Save"), this);
	saveAct->setShortcut(tr("Ctrl+S"));
	connect(saveAct, SIGNAL(triggered()), this, SLOT(slot_save()));

	importAct = new QAction(tr("&Import NifTI.."), this);
	importAct->setShortcut(tr("Ctrl+I"));
	connect(importAct, SIGNAL(triggered()), this, SLOT(slot_import()));

	undoAct = new QAction(tr("Undo"), this);
	undoAct->setShortcut(tr("Ctrl+Z"));
	connect(undoAct, SIGNAL(triggered()), mouseInteractor, SLOT(slot_undo()));

	redoAct = new QAction(tr("Redo"), this);
	redoAct->setShortcut(tr("Ctrl+Shift+Z"));
	connect(redoAct, SIGNAL(triggered()), mouseInteractor, SLOT(slot_redo()));
}

void EventQtSlotConnect::createMenus()
{
	fileMenu = new QMenu(tr("&File"), this);
	fileMenu->addAction(openAct);
	fileMenu->addAction(saveAct);
	fileMenu->addAction(importAct);
	menuBar()->addMenu(fileMenu);

	fileEdit = new QMenu(tr("Edit"), this);
	fileEdit->addAction(undoAct);
	fileEdit->addAction(redoAct);
	menuBar()->addMenu(fileEdit);
}

QComboBox* EventQtSlotConnect::getTagComboBox(){
	return this->comboBoxTagPoint;
}

void EventQtSlotConnect::readCustomDataLabel(vtkFloatArray* labelDBL)
{
	Global::labelData.clear();
	for(int i = 0; i < labelDBL->GetSize() - 1; i++)
	{
		Global::labelData.push_back(labelDBL->GetValue(i));
	}
}

void EventQtSlotConnect::readCustomDataTri(vtkFloatArray* triDBL)
{
	for(vtkIdType i = 0; i < triDBL->GetSize() - 1;)
	{
		vtkSmartPointer<vtkPoints> pts =
			vtkSmartPointer<vtkPoints>::New();

		TagTriangle tri;
		tri.p1[0] = triDBL->GetValue(i);tri.p1[1] = triDBL->GetValue(i+1);tri.p1[2] = triDBL->GetValue(i+2);
		tri.id1 = triDBL->GetValue(i+3);
		tri.seq1 = triDBL->GetValue(i+4);
		tri.p2[0] = triDBL->GetValue(i+5);tri.p2[1] = triDBL->GetValue(i+6);tri.p2[2] = triDBL->GetValue(i+7);
		tri.id2 = triDBL->GetValue(i+8);
		tri.seq2 = triDBL->GetValue(i+9);
		tri.p3[0] = triDBL->GetValue(i+10);tri.p3[1] = triDBL->GetValue(i+11);tri.p3[2] = triDBL->GetValue(i+12);
		tri.id3 = triDBL->GetValue(i+13);
		tri.seq3 = triDBL->GetValue(i+14);
		tri.index = triDBL->GetValue(i+15);

		for(int j = 0; j < 3; j++){
			double t1,t2,t3;
			t1 = triDBL->GetValue(i); t2 = triDBL->GetValue(i+1); t3 = triDBL->GetValue(i+2);
			pts->InsertNextPoint(t1,t2,t3);
			i += 5;
		}
		i += 1;

		vtkSmartPointer<vtkTriangle> triangle =
			vtkSmartPointer<vtkTriangle>::New();
		triangle->GetPointIds()->SetId ( 0, 0 );
		triangle->GetPointIds()->SetId ( 1, 1 );
		triangle->GetPointIds()->SetId ( 2, 2 );

		vtkSmartPointer<vtkCellArray> triangles =
			vtkSmartPointer<vtkCellArray>::New();
		triangles->InsertNextCell ( triangle );

		// Create a polydata object
		vtkSmartPointer<vtkPolyData> trianglePolyData =
			vtkSmartPointer<vtkPolyData>::New();

		// Add the geometry and topology to the polydata
		trianglePolyData->SetPoints ( pts );
		trianglePolyData->SetPolys ( triangles );

		// Create mapper and actor
		vtkSmartPointer<vtkPolyDataMapper> mapper =
			vtkSmartPointer<vtkPolyDataMapper>::New();
#if VTK_MAJOR_VERSION <= 5
		mapper->SetInput(trianglePolyData);
#else
		mapper->SetInputData(trianglePolyData);
#endif
		vtkSmartPointer<vtkActor> actor =
			vtkSmartPointer<vtkActor>::New();
		actor->SetMapper(mapper);
		actor->GetProperty()->SetColor(triLabelColors[tri.index].red()/255.0, 
			triLabelColors[tri.index].green()/255.0, 
			triLabelColors[tri.index].blue()/255.0);
		actor->GetProperty()->EdgeVisibilityOn();
		actor->GetProperty()->SetEdgeColor(0.0,0.0,0.0);
		vtkSmartPointer<vtkProperty> backPro = 
			vtkSmartPointer<vtkProperty>::New();
		backPro->SetColor(Global::backCol);
		actor->SetBackfaceProperty(backPro);

		tri.centerPos[0] = actor->GetCenter()[0];
		tri.centerPos[1] = actor->GetCenter()[1];
		tri.centerPos[2] = actor->GetCenter()[2];
		tri.triActor = actor;
		Global::vectorTagTriangles.push_back(tri);
		this->qvtkWidget->GetRenderWindow()->GetRenderers()->GetFirstRenderer()->AddActor(actor);
	}
}

void EventQtSlotConnect::readCustomDataEdge(vtkFloatArray* edgeDBL)
{
	for(int i = 0; i < edgeDBL->GetSize() - 1; i += 5)
	{
		TagEdge edge;
		edge.ptId1 = edgeDBL->GetValue(i);
		edge.ptId2 = edgeDBL->GetValue(i+1);
		edge.seq = edgeDBL->GetValue(i+2);
		edge.numEdge = edgeDBL->GetValue(i+3);
		edge.constrain = edgeDBL->GetValue(i+4);

		Global::vectorTagEdges.push_back(edge);
	}
}

void EventQtSlotConnect::readCustomDataTag(vtkFloatArray* tagDBL, vtkStringArray* tagStr)
{
	for(int i = 0, j = 0; i < tagDBL->GetSize() - 1; i += 5, j++)
	{
		TagInfo info;
		info.tagType = tagDBL->GetValue(i);
		info.tagIndex = tagDBL->GetValue(i+1);
		info.tagColor[0] = tagDBL->GetValue(i+2);
		info.tagColor[1] = tagDBL->GetValue(i+3);
		info.tagColor[2] = tagDBL->GetValue(i+4);
		info.qc = QColor(info.tagColor[0], info.tagColor[1], info.tagColor[2]);
		info.tagName = tagStr->GetValue(j).c_str();
		Global::vectorTagInfo.push_back(info);

		QPixmap pix(22,22);
		QString displayText = QString::number(info.tagIndex) + QString::fromStdString(" ") + (tagStr->GetValue(j));
		pix.fill(info.qc);
		this->comboBoxTagPoint->addItem(pix, displayText);
	}
}

void EventQtSlotConnect::readCustomDataTriLabel(vtkFloatArray* tagTriDBL, vtkStringArray* tagTriStr)
{
	for (int i = 0, j = 0; i < tagTriDBL->GetSize() - 1; i += 3, j++)
	{
		LabelTriangle lt;

		QPixmap pix(22, 22);
		QColor qc = QColor(tagTriDBL->GetValue(i), tagTriDBL->GetValue(i+1), tagTriDBL->GetValue(i+2));

		lt.labelName = tagTriStr->GetValue(j).c_str();
		lt.labelColor = qc;
		Global::vectorLabelInfo.push_back(lt);
		triLabelColors.push_back(qc);
		hideTriLabel.push_back(0);
		mouseInteractor->triLabelColors.push_back(qc);

		pix.fill(qc);
		this->TriLabelComboBox->addItem(pix, QString::fromUtf8(tagTriStr->GetValue(j)));
		mouseInteractor->currentTriIndex = 0;
	}
}

void EventQtSlotConnect::readCustomDataPoints(vtkFloatArray* ptsDBL)
{
	for(int i = 0; i < ptsDBL->GetSize() - 1; i += 7)
	{
		TagPoint tagPt;
		tagPt.pos[0] = ptsDBL->GetValue(i);
		tagPt.pos[1] = ptsDBL->GetValue(i+1);
		tagPt.pos[2] = ptsDBL->GetValue(i+2);
		tagPt.radius = ptsDBL->GetValue(i+3);
		tagPt.seq = ptsDBL->GetValue(i+4);
		tagPt.typeIndex = ptsDBL->GetValue(i+5);
		tagPt.comboBoxIndex = ptsDBL->GetValue(i+6);

		//Create a sphere
		vtkSmartPointer<vtkSphereSource> sphereSource =
			vtkSmartPointer<vtkSphereSource>::New();
		sphereSource->SetCenter(tagPt.pos[0], tagPt.pos[1], tagPt.pos[2]);
		sphereSource->SetRadius(1.0);

		//Create a mapper and actor
		vtkSmartPointer<vtkPolyDataMapper> mapper =
			vtkSmartPointer<vtkPolyDataMapper>::New();
		mapper->SetInputConnection(sphereSource->GetOutputPort());

		//QComboBox* cbTagPoint =  qtObject->getTagComboBox();
		TagInfo ti = Global::vectorTagInfo[tagPt.comboBoxIndex];
		vtkSmartPointer<vtkActor> actor =
			vtkSmartPointer<vtkActor>::New();
		actor->SetMapper(mapper);
		actor->GetProperty()->SetColor(ti.tagColor[0] / 255.0, ti.tagColor[1] / 255.0, ti.tagColor[2] / 255.0);

		tagPt.actor = actor;
		Global::vectorTagPoints.push_back(tagPt);
		this->qvtkWidget->GetRenderWindow()->GetRenderers()->GetFirstRenderer()->AddActor(actor);
	}
}

void EventQtSlotConnect::readCustomData(vtkPolyData *polydata)
{
	vtkFloatArray* labelDBL = (vtkFloatArray*)polydata->GetFieldData()->GetArray("Label");
	if (labelDBL != NULL)
		readCustomDataLabel(labelDBL);

	vtkFloatArray* tagDBL = (vtkFloatArray*)polydata->GetFieldData()->GetArray("TagInfo");
	vtkStringArray* tagStr = (vtkStringArray*)polydata->GetFieldData()->GetAbstractArray("TagName");
	if (tagStr != NULL){
		std::cout << "string size " << tagStr->GetSize() - 1 << std::endl;
		readCustomDataTag(tagDBL, tagStr);
	}

	vtkFloatArray* ptsDBL = (vtkFloatArray*)polydata->GetFieldData()->GetArray("TagPoints");
	if (ptsDBL != NULL) {
		std::cout << "after tag point" << std::endl;
		readCustomDataPoints(ptsDBL);
	}

	vtkFloatArray* tagTriDBL = (vtkFloatArray*)polydata->GetFieldData()->GetArray("LabelTriangleColor");
	vtkStringArray* tagTriStr = (vtkStringArray*)polydata->GetFieldData()->GetAbstractArray("LabelTriangleName");
	if (tagTriStr != NULL) {
		std::cout << "label size " << tagTriStr->GetSize() << std::endl;
		readCustomDataTriLabel(tagTriDBL, tagTriStr);
	}

	vtkFloatArray* triDBL = (vtkFloatArray*)polydata->GetFieldData()->GetArray("TagTriangles");
	if (triDBL != NULL) {
		std::cout << "after tri point" << std::endl;
		readCustomDataTri(triDBL);
	}
	
	vtkFloatArray* edgeDBL = (vtkFloatArray*)polydata->GetFieldData()->GetArray("TagEdges");
	if (edgeDBL != NULL) {
		std::cout << "after tagEdge point" << std::endl;
		readCustomDataEdge(edgeDBL);
	}
}

void EventQtSlotConnect::readVTK(std::string filename){
	std::string inputFilename = filename;

	vtkSmartPointer<vtkGenericDataObjectReader> reader = 
	  vtkSmartPointer<vtkGenericDataObjectReader>::New();
    reader->SetFileName(inputFilename.c_str());
    reader->Update();

	// Create a polydata object
    vtkPolyData* polydata =  reader->GetPolyDataOutput();	

	//initialize label data for store in vtk file
	this->polyObject = polydata;

	//see if skeleton vtk
	vtkDataArray* skelVtk = polydata->GetPointData()->GetArray("Radius");
	if(skelVtk == NULL)
		Global::isSkeleton = false;
	else
		Global::isSkeleton = true;

	Global::labelData.clear();
	Global::labelData.resize(polydata->GetPoints()->GetNumberOfPoints());
	for(int i = 0; i < Global::labelData.size(); i++)
		Global::labelData[i] = 0.0;

	// Create a mapper
	vtkSmartPointer<vtkPolyDataMapper> mapper =
		vtkSmartPointer<vtkPolyDataMapper>::New();
#if VTK_MAJOR_VERSION <= 5
	mapper->SetInput ( polydata );
#else
	mapper->SetInputData ( polydata );
#endif

	// Create an actor
	vtkSmartPointer<vtkActor> actor =
		vtkSmartPointer<vtkActor>::New();
	actor->SetMapper ( mapper );

	std::cout << "Actor address: " << actor << std::endl;

	// A renderer and render window
	vtkSmartPointer<vtkRenderer> renderer =
		vtkSmartPointer<vtkRenderer>::New();
	this->qvtkWidget->GetRenderWindow()->SetAlphaBitPlanes(1);
	this->qvtkWidget->GetRenderWindow()->SetMultiSamples(0);
	renderer->SetUseDepthPeeling(1);
	renderer->SetMaximumNumberOfPeels(4);
	renderer->SetOcclusionRatio(0.1);
	this->qvtkWidget->GetRenderWindow()->GetInteractor()->SetRenderWindow(this->qvtkWidget->GetRenderWindow());
	
	//renderWindowInteractor->SetInteractorStyle( style );	
	renderer->AddActor ( actor );
	

	renderer->SetBackground ( 0,0,1 );	
	renderer->ResetCamera();

	//Set the custom style to use for interaction.
	mouseInteractor->SetDefaultRenderer(renderer);
	mouseInteractor->labelTriNumber = this->TriangleNumber;
	mouseInteractor->labelPtNumber = this->PointNumber;
    /*mouseInteractor->undoButton = this->pushButtonUndo;
    mouseInteractor->redoButton = this->pushButtonRedo;*/
	
	//reset everything
	Global::vectorTagPoints.clear();
	Global::vectorTagTriangles.clear();
	Global::vectorTagEdges.clear();
	Global::vectorTagInfo.clear();
	Global::triNormalActors.clear();
	Global::selectedTag = 0;
	Global::vectorLabelInfo.clear();
	mouseInteractor->triLabelColors.clear();
	triLabelColors.clear();
	hideTriLabel.clear();
	this->comboBoxTagPoint->clear();
	this->checkBoxHideMesh->setChecked(false);
	this->checkBoxHideSkel->setChecked(false);
	this->TriLabelComboBox->clear();
	mouseInteractor->operationFlag = VIEW;
	setToolButton(VIEW);

	this->qvtkWidget->GetRenderWindow()->GetInteractor()->SetInteractorStyle( mouseInteractor );
	if(this->qvtkWidget->GetRenderWindow()->GetRenderers()->GetFirstRenderer() != NULL)
		this->qvtkWidget->GetRenderWindow()->RemoveRenderer(this->qvtkWidget->GetRenderWindow()->GetRenderers()->GetFirstRenderer());
	this->qvtkWidget->GetRenderWindow()->AddRenderer(renderer);
    this->qvtkWidget->update();
   /* this->labelBckgndColor->setPalette(QPalette(Qt::blue));
    this->labelBckgndColor->setAutoFillBackground(true);*/

	//get normals
	vtkSmartPointer<vtkPolyDataNormals> normalGenerator = vtkSmartPointer<vtkPolyDataNormals>::New();
	/*vtkSmartPointer<vtkActor> actor0 = static_cast<vtkActor *>(this->GetDefaultRenderer()->GetActors()->GetItemAsObject(0));
	vtkSmartPointer<vtkDataSet> vtkdata = actor0->GetMapper()->GetInputAsDataSet();	*/

#if VTK_MAJOR_VERSION <= 5
	normalGenerator->SetInput(polydata);
#else
	normalGenerator->SetInputData(polydata);
#endif
	normalGenerator->ComputePointNormalsOn();
	normalGenerator->ComputeCellNormalsOff();
	normalGenerator->Update();

	mouseInteractor->setNormalGenerator(normalGenerator);

	//see triangle
	//vtkDoubleArray* triDBL = (vtkDoubleArray*)polydata->GetFieldData()->GetArray("TagPoints");
	vtkDoubleArray* triDBL = (vtkDoubleArray*)polydata->GetFieldData()->GetArray("TagInfo");

	if (triDBL != NULL)
	{
		readCustomData(polydata);
	}

	this->PointNumber->setText(QString::number(Global::vectorTagPoints.size()));
	this->TriangleNumber->setText(QString::number(Global::vectorTagTriangles.size()));
    /*this->pushButtonUndo->setEnabled(false);
    this->pushButtonRedo->setEnabled(false);*/

	this->ViewToolButton->setEnabled(false);
	this->InflateCheckBox->setChecked(false);
}

itk::SmartPointer<itk::OrientedRASImage<double, 3>> EventQtSlotConnect::threshold(
	itk::SmartPointer<itk::OrientedRASImage<double, 3>> input, double u1, double u2, double v1, double v2)
{
	typedef itk::BinaryThresholdImageFilter<ImageType, ImageType> threshFilterType;
	typename threshFilterType::Pointer threshFilter = threshFilterType::New();
	threshFilter->SetInput(input);
	threshFilter->SetLowerThreshold(u1);
	threshFilter->SetUpperThreshold(u2);
	threshFilter->SetInsideValue(v1);
	threshFilter->SetOutsideValue(v2);
	threshFilter->Update();

	return threshFilter->GetOutput();
}

itk::SmartPointer<itk::OrientedRASImage<double, 3>> EventQtSlotConnect::smooth(
	itk::SmartPointer<itk::OrientedRASImage<double, 3>> input, const char * sigma)
{
	vnl_vector_fixed<double, 3> var;
	var.fill(atof(sigma));

	typename ImageType::TransformMatrixType M = input->GetVoxelSpaceToRASPhysicalSpaceMatrix();

	vnl_vector_fixed<double, 3 + 1> X, XP;
	for (size_t d = 0; d < 3; d++)
		X[d] = var[d];
	X[3] = 0.0;

	// Apply matrix
	XP = M * X;
	for (size_t d = 0; d < 3; d++)
		var[d] = XP[d];

	for (size_t d = 0; d < 3; d++)
		var[d] = fabs(var[d]);

	// Creation of the gaussian filter
	typedef itk::DiscreteGaussianImageFilter< ImageType, ImageType >  smoothFilterType;
	smoothFilterType::Pointer gaussianFilter = smoothFilterType::New();
	smoothFilterType::ArrayType variance;

	for (size_t i = 0; i < 3; i++)
		variance[i] = var[i] * var[i];

	// Smoothing operation
	gaussianFilter->SetInput(input);
	gaussianFilter->SetVariance(variance);
	gaussianFilter->SetUseImageSpacingOn();
	gaussianFilter->Update();

	return gaussianFilter->GetOutput();
}

void EventQtSlotConnect::writeNiftii(
	itk::SmartPointer<itk::OrientedRASImage<double, 3>> input, const char * outputFilename)
{
	typedef itk::OrientedRASImage<double, 3> OutputImageType;
	typename OutputImageType::Pointer output = OutputImageType::New();
	output->SetRegions(input->GetBufferedRegion());
	output->SetSpacing(input->GetSpacing());
	output->SetOrigin(input->GetOrigin());
	output->SetDirection(input->GetDirection());
	output->SetMetaDataDictionary(input->GetMetaDataDictionary());
	output->Allocate();

	// Copy everything, rounding if the pixel type is integer
	size_t n = input->GetBufferedRegion().GetNumberOfPixels();
	for (size_t i = 0; i < n; i++)
		output->GetBufferPointer()[i] = (double)(input->GetBufferPointer()[i] + 0.0);

	// Set the file notes for this image
	itk::EncapsulateMetaData<string>(output->GetMetaDataDictionary(), itk::ITK_FileNotes, std::string("Created by Convert3D"));

	// Write the image out
	typedef itk::ImageFileWriter<OutputImageType> WriterType;
	typename WriterType::Pointer writer = WriterType::New();
	writer->SetInput(output);

	writer->SetFileName(outputFilename);
	writer->Update();
}

void EventQtSlotConnect::importNIFTI(std::vector<std::string> filenames, bool checked, std::string sigma, std::vector<std::string> th1Param, std::vector<std::string> th2Param)
{
	// Parameters 
	const char *inputFilename = filenames[0].c_str();
	const char *outputFilename = filenames[1].c_str();
	
	if (checked)
	{
		const char *sigmaVal = sigma.c_str();

		// First threshold parameters
		double u11 = strcmp(th1Param[0].c_str(), "-inf") == 0 ? -vnl_huge_val(0.0) : std::stod(th1Param[0]);
		double u21 = strcmp(th1Param[1].c_str(), "inf") == 0 ? vnl_huge_val(0.0) : std::stod(th1Param[1]);
		double v11 = std::stod(th1Param[2]);
		double v21 = std::stod(th1Param[3]);

		// Second threshold parameters
		double u12 = strcmp(th2Param[0].c_str(), "-inf") == 0 ? -vnl_huge_val(0.0) : std::stod(th2Param[0]);
		double u22 = strcmp(th2Param[1].c_str(), "inf") == 0 ? vnl_huge_val(0.0) : std::stod(th2Param[1]);
		double v12 = std::stod(th2Param[2]);
		double v22 = std::stod(th2Param[3]);

		//Read the nifti file
		typename itk::ImageIOBase::Pointer iobase;
		iobase = itk::ImageIOFactory::CreateImageIO(inputFilename, itk::ImageIOFactory::ReadMode);
		iobase->SetFileName(inputFilename);

		// Read the image information
		iobase->ReadImageInformation();

		// Set up the reader
		typedef itk::ImageFileReader<ImageType> ReaderType;
		typename ReaderType::Pointer reader = ReaderType::New();
		reader->SetFileName(inputFilename);
		reader->SetImageIO(iobase);

		// nii input -Thresholding 1-> -Smoothing-> -Thresholding 2-> nii temp output
		ImagePointer input_threshold1 = reader->GetOutput();
		ImagePointer input_smooth = threshold(input_threshold1, u11, u21, v11, v21);
		ImagePointer input_threshold2 = smooth(input_smooth, sigmaVal);
		ImagePointer outputtemp = threshold(input_threshold2, u12, u22, v12, v22);

		// Create the output image 
		const char *niiTemp = "temp.nii.gz";
		writeNiftii(outputtemp, niiTemp);
		vtklevelset(niiTemp, outputFilename, "1");
		remove(niiTemp);
		std::cout << "Smoothing and conversion done" << std::endl;
	}
	else
	{
		vtklevelset(inputFilename, outputFilename, "1");
		std::cout << "Conversion done" << std::endl;
	}
	VTKfilename = std::string(outputFilename);
	readVTK(VTKfilename);
}

void EventQtSlotConnect::vtklevelset(const char * inputNii, const char * outputVtk, std::string threshold)
{
	// Read the input image
	typedef itk::OrientedRASImage<float, 3> ImageType;
	typedef itk::ImageFileReader<ImageType> ReaderType;
	ReaderType::Pointer fltReader = ReaderType::New();
	fltReader->SetFileName(inputNii);
	fltReader->Update();
	ImageType::Pointer imgInput = fltReader->GetOutput();

	// Get the range of the input image
	float imax = imgInput->GetBufferPointer()[0];
	float imin = imax;
	for (size_t i = 0; i < imgInput->GetBufferedRegion().GetNumberOfPixels(); i++)
	{
		float x = imgInput->GetBufferPointer()[i];
		imax = std::max(imax, x);
		imin = std::min(imin, x);
	}

	float cut = atof(threshold.c_str());
	std::cout << "Image Range: [" << imin << ", " << imax << "]" << std::endl;
	std::cout << "Taking level set at " << cut << std::endl;

	// Create an importer and an exporter in VTK
	typedef itk::VTKImageExport<ImageType> ExporterType;
	ExporterType::Pointer fltExport = ExporterType::New();
	fltExport->SetInput(imgInput);
	vtkImageImport *fltImport = vtkImageImport::New();

	//ConnectITKToVTK(fltExport.GetPointer(), fltImport);
	fltImport->SetUpdateInformationCallback(fltExport.GetPointer()->GetUpdateInformationCallback());
	fltImport->SetPipelineModifiedCallback(fltExport.GetPointer()->GetPipelineModifiedCallback());
	fltImport->SetWholeExtentCallback(fltExport.GetPointer()->GetWholeExtentCallback());
	fltImport->SetSpacingCallback(fltExport.GetPointer()->GetSpacingCallback());
	fltImport->SetOriginCallback(fltExport.GetPointer()->GetOriginCallback());
	fltImport->SetScalarTypeCallback(fltExport.GetPointer()->GetScalarTypeCallback());
	fltImport->SetNumberOfComponentsCallback(fltExport.GetPointer()->GetNumberOfComponentsCallback());
	fltImport->SetPropagateUpdateExtentCallback(fltExport.GetPointer()->GetPropagateUpdateExtentCallback());
	fltImport->SetUpdateDataCallback(fltExport.GetPointer()->GetUpdateDataCallback());
	fltImport->SetDataExtentCallback(fltExport.GetPointer()->GetDataExtentCallback());
	fltImport->SetBufferPointerCallback(fltExport.GetPointer()->GetBufferPointerCallback());
	fltImport->SetCallbackUserData(fltExport.GetPointer()->GetCallbackUserData());

	// Run marching cubes on the input image
	vtkMarchingCubes *fltMarching = vtkMarchingCubes::New();
	fltMarching->SetInputConnection(fltImport->GetOutputPort());
	fltMarching->ComputeScalarsOff();
	fltMarching->ComputeGradientsOff();
	fltMarching->ComputeNormalsOn();
	fltMarching->SetNumberOfContours(1);
	fltMarching->SetValue(0, cut);
	fltMarching->Update();

	vtkPolyData *pipe_tail = fltMarching->GetOutput();

	// Create the transform filter
	vtkTransformPolyDataFilter *fltTransform = vtkTransformPolyDataFilter::New();
	fltTransform->SetInputData(pipe_tail);

	// Compute the transform from VTK coordinates to NIFTI/RAS coordinates
	typedef vnl_matrix_fixed<double, 4, 4> Mat44;
	Mat44 vtk2out;
	Mat44 vtk2nii = ConstructVTKtoNiftiTransform(
		imgInput->GetDirection().GetVnlMatrix(),
		imgInput->GetOrigin().GetVnlVector(),
		imgInput->GetSpacing().GetVnlVector());

	// If we actually asked for voxel coordinates, we need to fix that

	vtk2out = vtk2nii;
	

	// Update the VTK transform to match
	vtkTransform *transform = vtkTransform::New();
	transform->SetMatrix(vtk2out.data_block());
	fltTransform->SetTransform(transform);
	fltTransform->Update();

	// Get final output
	vtkPolyData *mesh = fltTransform->GetOutput();

	// Flip normals if determinant of SFORM is negative
	if (transform->GetMatrix()->Determinant() < 0)
	{
		vtkPointData *pd = mesh->GetPointData();
		vtkDataArray *nrm = pd->GetNormals();
		for (size_t i = 0; i < (size_t)nrm->GetNumberOfTuples(); i++)
			for (size_t j = 0; j < (size_t)nrm->GetNumberOfComponents(); j++)
				nrm->SetComponent(i, j, -nrm->GetComponent(i, j));
		nrm->Modified();
	}

	// Write the output
	vtkPolyDataWriter *writer = vtkPolyDataWriter::New();
	writer->SetFileName(outputVtk);		
	writer->SetInputData(mesh);
	writer->Update();
}

void EventQtSlotConnect::saveVTKFile(QString fileName)
{
	vtkSmartPointer<vtkGenericDataObjectWriter> writer = 
		vtkSmartPointer<vtkGenericDataObjectWriter>::New();

#ifdef _WIN64
	writer->SetFileName((fileName.toStdString().substr(0, fileName.toStdString().length() - 4)).append("Affix.vtk").c_str());	
#elif _WIN32
	writer->SetFileName((fileName.toStdString().substr(0, fileName.toStdString().length() - 4)).append("Affix.vtk").c_str());	
#elif __APPLE__
	writer->SetFileName((fileName.toStdString().substr(0, fileName.toStdString().length() - 4)).append("Affix.vtk").c_str());
#elif __linux__
	writer->SetFileName((fileName.toStdString().substr(0, fileName.toStdString().length() - 4)).append("Affix.vtk").c_str());
#endif
	
	vtkSmartPointer<vtkPolyData> finalPolyData =
		vtkSmartPointer<vtkPolyData>::New();

	finalPolyData = polyObject;

	if(finalPolyData->GetFieldData()->GetArray("Label")){
		finalPolyData->GetFieldData()->RemoveArray("Label");
	}
	if (finalPolyData->GetFieldData()->GetArray("LabelTriangleName")) {
		finalPolyData->GetFieldData()->RemoveArray("LabelTriangleName");
	}
	if (finalPolyData->GetFieldData()->GetArray("LabelTriangleColor")) {
		finalPolyData->GetFieldData()->RemoveArray("LabelTriangleColor");
	}
	if(finalPolyData->GetFieldData()->GetArray("TagTriangles")){
		finalPolyData->GetFieldData()->RemoveArray("TagTriangles");
	}
	if(finalPolyData->GetFieldData()->GetArray("TagEdges")){
		finalPolyData->GetFieldData()->RemoveArray("TagEdges");
	}
	if(finalPolyData->GetFieldData()->GetArray("TagPoints")){
		finalPolyData->GetFieldData()->RemoveArray("TagPoints");
	}
	if(finalPolyData->GetFieldData()->GetArray("TagInfo")){
		finalPolyData->GetFieldData()->RemoveArray("TagInfo");
	}
	
	vtkSmartPointer<vtkFieldData> field =
		vtkSmartPointer<vtkFieldData>::New();

	vtkSmartPointer<vtkFloatArray> fltArray1 = 
		vtkSmartPointer<vtkFloatArray>::New();
	fltArray1->SetName("Label");
	for(int i = 0; i < Global::labelData.size(); i++)
		fltArray1->InsertNextValue(Global::labelData[i]);
	if(Global::labelData.size() !=0 )
		//finalPolyData->GetFieldData()->AddArray(fltArray1);
		field->AddArray(fltArray1);

	vtkSmartPointer<vtkStringArray> strArray2_1 =
		vtkSmartPointer<vtkStringArray>::New();
	strArray2_1->SetName("LabelTriangleName");

	vtkSmartPointer<vtkFloatArray> fltArray2_1 =
		vtkSmartPointer<vtkFloatArray>::New();
	fltArray2_1->SetName("LabelTriangleColor");

	for (int i = 0; i < Global::vectorLabelInfo.size(); i++) {
		strArray2_1->InsertNextValue(Global::vectorLabelInfo[i].labelName.c_str());
		fltArray2_1->InsertNextValue(Global::vectorLabelInfo[i].labelColor.red());
		fltArray2_1->InsertNextValue(Global::vectorLabelInfo[i].labelColor.green());
		fltArray2_1->InsertNextValue(Global::vectorLabelInfo[i].labelColor.blue());
	}

	if (Global::vectorLabelInfo.size() != 0) {
		field->AddArray(strArray2_1);
		field->AddArray(fltArray2_1);
	}

	vtkSmartPointer<vtkFloatArray> fltArray2 = 
		vtkSmartPointer<vtkFloatArray>::New();
	fltArray2->SetName("TagTriangles");
	for(int i = 0; i < Global::vectorTagTriangles.size(); i++)
	{
		fltArray2->InsertNextValue(Global::vectorTagTriangles[i].p1[0]);
		fltArray2->InsertNextValue(Global::vectorTagTriangles[i].p1[1]);
		fltArray2->InsertNextValue(Global::vectorTagTriangles[i].p1[2]);
		fltArray2->InsertNextValue(Global::vectorTagTriangles[i].id1);
		fltArray2->InsertNextValue(Global::vectorTagTriangles[i].seq1);
		fltArray2->InsertNextValue(Global::vectorTagTriangles[i].p2[0]);
		fltArray2->InsertNextValue(Global::vectorTagTriangles[i].p2[1]);
		fltArray2->InsertNextValue(Global::vectorTagTriangles[i].p2[2]);
		fltArray2->InsertNextValue(Global::vectorTagTriangles[i].id2);
		fltArray2->InsertNextValue(Global::vectorTagTriangles[i].seq2);
		fltArray2->InsertNextValue(Global::vectorTagTriangles[i].p3[0]);
		fltArray2->InsertNextValue(Global::vectorTagTriangles[i].p3[1]);
		fltArray2->InsertNextValue(Global::vectorTagTriangles[i].p3[2]);
		fltArray2->InsertNextValue(Global::vectorTagTriangles[i].id3);
		fltArray2->InsertNextValue(Global::vectorTagTriangles[i].seq3);
		fltArray2->InsertNextValue(Global::vectorTagTriangles[i].index);
	}
	if(Global::vectorTagTriangles.size() != 0)
		//finalPolyData->GetFieldData()->AddArray(fltArray2);
		field->AddArray(fltArray2);

	vtkSmartPointer<vtkFloatArray> fltArray3 = 
		vtkSmartPointer<vtkFloatArray>::New();
	fltArray3->SetName("TagEdges");
	for(int i = 0; i < Global::vectorTagEdges.size(); i++)
	{
		fltArray3->InsertNextValue(Global::vectorTagEdges[i].ptId1);
		fltArray3->InsertNextValue(Global::vectorTagEdges[i].ptId2);
		fltArray3->InsertNextValue(Global::vectorTagEdges[i].seq);
		fltArray3->InsertNextValue(Global::vectorTagEdges[i].numEdge);
		fltArray3->InsertNextValue(Global::vectorTagEdges[i].constrain);
	}
	if(Global::vectorTagEdges.size() != 0)
		//finalPolyData->GetFieldData()->AddArray(fltArray3);
		field->AddArray(fltArray3);

	vtkSmartPointer<vtkFloatArray> fltArray4 = 
		vtkSmartPointer<vtkFloatArray>::New();
	fltArray4->SetName("TagPoints");
	for(int i = 0; i < Global::vectorTagPoints.size(); i++)
	{
		fltArray4->InsertNextValue(Global::vectorTagPoints[i].pos[0]);
		fltArray4->InsertNextValue(Global::vectorTagPoints[i].pos[1]);
		fltArray4->InsertNextValue(Global::vectorTagPoints[i].pos[2]);
		fltArray4->InsertNextValue(Global::vectorTagPoints[i].radius);
		fltArray4->InsertNextValue(Global::vectorTagPoints[i].seq);
		fltArray4->InsertNextValue(Global::vectorTagPoints[i].typeIndex);
		fltArray4->InsertNextValue(Global::vectorTagPoints[i].comboBoxIndex);
	}
	if(Global::vectorTagPoints.size() != 0)
		//finalPolyData->GetFieldData()->AddArray(fltArray4);
		field->AddArray(fltArray4);

	vtkSmartPointer<vtkFloatArray> fltArray5 = 
		vtkSmartPointer<vtkFloatArray>::New();
	fltArray5->SetName("TagInfo");

	vtkSmartPointer<vtkStringArray> strArray1 = 
		vtkSmartPointer<vtkStringArray>::New();
	strArray1->SetName("TagName");

	for(int i = 0; i < Global::vectorTagInfo.size(); i++)
	{
		fltArray5->InsertNextValue(Global::vectorTagInfo[i].tagType);
		fltArray5->InsertNextValue(Global::vectorTagInfo[i].tagIndex);
		fltArray5->InsertNextValue(Global::vectorTagInfo[i].tagColor[0]);
		fltArray5->InsertNextValue(Global::vectorTagInfo[i].tagColor[1]);
		fltArray5->InsertNextValue(Global::vectorTagInfo[i].tagColor[2]);

		strArray1->InsertNextValue(Global::vectorTagInfo[i].tagName.c_str());
	}
	if(Global::vectorTagInfo.size() != 0)
	{
		field->AddArray(fltArray5);
		field->AddArray(strArray1);
	}


	vtkSmartPointer<vtkIntArray> intArray1 = 
		vtkSmartPointer<vtkIntArray>::New();
	intArray1->SetName("TriSeq");
	for(int i = 0; i < Global::vectorTagTriangles.size(); i++)
	{
		intArray1->InsertNextValue(Global::vectorTagTriangles[i].seq1);
		intArray1->InsertNextValue(Global::vectorTagTriangles[i].seq2);
		intArray1->InsertNextValue(Global::vectorTagTriangles[i].seq3);
	}
	//finalPolyData->GetFieldData()->AddArray(intArray1);
	field->AddArray(intArray1);

	finalPolyData->SetFieldData(field);
    writer->SetInputData(finalPolyData);
	writer->Update();
	writer->Write();
}

void EventQtSlotConnect::saveParaViewFile(QString fileName)
{
    fileName.replace(".vtk","");
	if(Global::vectorTagTriangles.size() > 0)
	{
		//////////////save another file for ParaView////////////////////
		vtkSmartPointer<vtkGenericDataObjectWriter> writerParaView = 
			vtkSmartPointer<vtkGenericDataObjectWriter>::New();
#ifdef _WIN64
		writerParaView->SetFileName(fileName.toStdString().append(".vtk").c_str());
#elif _WIN32
		writerParaView->SetFileName(fileName.toStdString().append(".vtk").c_str());
#elif __APPLE__
		writerParaView->SetFileName(fileName.toStdString().append(".vtk").c_str());
#elif __linux__
		writerParaView->SetFileName(fileName.toStdString().append(".vtk").c_str());
#endif


		//Append the two meshes 
		vtkSmartPointer<vtkAppendPolyData> appendFilter =
			vtkSmartPointer<vtkAppendPolyData>::New();

		for(int i = 0; i < Global::vectorTagTriangles.size(); i++)
		{
			vtkSmartPointer<vtkActorCollection> actorCollection =
				vtkSmartPointer<vtkActorCollection>::New();
			Global::vectorTagTriangles[i].triActor->GetActors(actorCollection);		
			vtkPolyData* polyData = vtkPolyData::SafeDownCast(actorCollection->GetLastActor()->GetMapper()->GetInput());
            appendFilter->AddInputData(polyData);
		}

		vtkSmartPointer<vtkCleanPolyData> cleanPoly = 
			vtkSmartPointer<vtkCleanPolyData>::New();

        cleanPoly->SetInputConnection(appendFilter->GetOutputPort());
		cleanPoly->Update();

		std::vector<int> labelData;
		std::vector<double> radiusData;

		for(int i = 0; i < cleanPoly->GetOutput()->GetNumberOfPoints(); i++)
		{
			for(int j = 0; j < Global::vectorTagPoints.size(); j++)
			{
				if(Global::vectorTagPoints[j].pos[0] == cleanPoly->GetOutput()->GetPoint(i)[0] &&
					Global::vectorTagPoints[j].pos[1] == cleanPoly->GetOutput()->GetPoint(i)[1] &&
					Global::vectorTagPoints[j].pos[2] == cleanPoly->GetOutput()->GetPoint(i)[2])
				{
					labelData.push_back(Global::vectorTagPoints[j].typeIndex);
					radiusData.push_back(Global::vectorTagPoints[j].radius);
				}
			}
		}

		vtkSmartPointer<vtkFloatArray> fltArray6 = 
			vtkSmartPointer<vtkFloatArray>::New();
		fltArray6->SetName("Label");
		for(int i = 0; i < labelData.size(); i++)
			fltArray6->InsertNextValue(labelData[i]);

		vtkSmartPointer<vtkFloatArray> fltArray7 = 
			vtkSmartPointer<vtkFloatArray>::New();
		fltArray7->SetName("Radius");
		for(int i = 0; i < radiusData.size(); i++)
			fltArray7->InsertNextValue(radiusData[i]);

		vtkSmartPointer<vtkFloatArray> fltArray8 =
			vtkSmartPointer<vtkFloatArray>::New();
		// fltArray8->SetName("TriLabel");
		fltArray8->SetName("Label");
		for(int i = 0; i < Global::vectorTagTriangles.size(); i++)
			fltArray8->InsertNextValue(Global::vectorTagTriangles[i].index + 1);

		cleanPoly->GetOutput()->GetPointData()->AddArray(fltArray6);
		cleanPoly->GetOutput()->GetPointData()->AddArray(fltArray7);
		cleanPoly->GetOutput()->GetCellData()->AddArray(fltArray8);
        writerParaView->SetInputData(cleanPoly->GetOutput());
		//writerParaView->SetFileTypeToBinary();//solve for matlab
		writerParaView->SetFileTypeToASCII();
		writerParaView->Update();
		writerParaView->Write();		
	}
		
}

void EventQtSlotConnect::exportSubdivideMesh(QString fileName)
{
	int numberOfSubdivisions = this->SubLevelComboBox->currentIndex();
	fileName.replace(".vtk", "");
	if (Global::vectorTagTriangles.size() > 0 && numberOfSubdivisions > 0)
	{
		//////////////save another file for ParaView////////////////////
		vtkSmartPointer<vtkGenericDataObjectWriter> writerSubParaView =
			vtkSmartPointer<vtkGenericDataObjectWriter>::New();
#ifdef _WIN64
		writerSubParaView->SetFileName(fileName.toStdString().append("Subdivide.vtk").c_str());
#elif _WIN32
		writerSubParaView->SetFileName(fileName.toStdString().append("Subdivide.vtk").c_str());
#elif __APPLE__
		writerSubParaView->SetFileName(fileName.toStdString().append("Subdivide.vtk").c_str());
#elif __linux__
		writerSubParaView->SetFileName(fileName.toStdString().append("Subdivide.vtk").c_str());
#endif
	
		//Append the two meshes 
		vtkSmartPointer<vtkAppendPolyData> appendFilter =
			vtkSmartPointer<vtkAppendPolyData>::New();

		for (int i = 0; i < Global::vectorTagTriangles.size(); i++)
		{
			vtkSmartPointer<vtkActorCollection> actorCollection =
				vtkSmartPointer<vtkActorCollection>::New();
			Global::vectorTagTriangles[i].triActor->GetActors(actorCollection);
			vtkPolyData* polyData = vtkPolyData::SafeDownCast(actorCollection->GetLastActor()->GetMapper()->GetInput());
			appendFilter->AddInputData(polyData);
		}

		vtkSmartPointer<vtkCleanPolyData> cleanPoly =
			vtkSmartPointer<vtkCleanPolyData>::New();

		cleanPoly->SetInputConnection(appendFilter->GetOutputPort());
		cleanPoly->Update();

		//std::vector<int> labelData;
		std::vector<double> radiusData;

		vtkSmartPointer<vtkLoopSubdivisionFilter> subdivisionFilter =
			vtkSmartPointer<vtkLoopSubdivisionFilter>::New();

		subdivisionFilter->SetNumberOfSubdivisions(numberOfSubdivisions);
		subdivisionFilter->SetInputData(cleanPoly->GetOutput());
		subdivisionFilter->Update();

		vtkSmartPointer<vtkActorCollection> actors = mouseInteractor->GetDefaultRenderer()->GetActors();
		vtkSmartPointer<vtkActor> actor0 = static_cast<vtkActor *>(actors->GetItemAsObject(0));
		vtkSmartPointer<vtkDataSet> vtkdata = actor0->GetMapper()->GetInputAsDataSet();
		vtkDoubleArray* radiusArray = (vtkDoubleArray*)vtkdata->GetPointData()->GetArray("Radius");
		vtkSmartPointer<vtkPolyData> outFilter = subdivisionFilter->GetOutput();

		for (int i = 0; i < subdivisionFilter->GetOutput()->GetNumberOfPoints(); i++)
		{
			double pos[3];
			outFilter->GetPoint(i, pos);

			//std::cout << "Point " << i << ": " << pos[0] << " "
			//	<< pos[1] << " "
			//	<< pos[2] << std::endl;

			double minDistance = DBL_MAX;
			double pointRadius = 0.0;  

			for (vtkIdType j = 0; j < vtkdata->GetNumberOfPoints(); j++) {
				double p[3];
				vtkdata->GetPoint(j, p);
				
				double dist = std::sqrt(std::pow(pos[0] - p[0], 2) + std::pow(pos[1] - p[1], 2) + std::pow(pos[2] - p[2], 2));
				//Find the closet vertex
				if (dist < minDistance) {
					minDistance = dist;
					pointRadius = radiusArray->GetValue(j);
				}
			}
			radiusData.push_back(pointRadius);
		}

		vtkSmartPointer<vtkFloatArray> fltArray7 =
			vtkSmartPointer<vtkFloatArray>::New();
		fltArray7->SetName("Radius");
		for (int i = 0; i < radiusData.size(); i++)
			fltArray7->InsertNextValue(radiusData[i]);

		subdivisionFilter->GetOutput()->GetPointData()->AddArray(fltArray7);
		writerSubParaView->SetInputData(subdivisionFilter->GetOutput());
		writerSubParaView->SetFileTypeToASCII();
		writerSubParaView->Update();
		writerSubParaView->Write();
	}
}

void EventQtSlotConnect::saveCmrepFile(QString fileName)
{
    fileName.replace(".vtk","");
	///////////////save cmrep file ////////////////////
	std::ofstream cmrepFile;
#ifdef _WIN64
	cmrepFile.open((fileName.toStdString()).append(".cmrep").c_str());
#elif _WIN32
	cmrepFile.open((fileName.toStdString()).append(".cmrep").c_str());
#elif __APPLE__
	cmrepFile.open((fileName.toStdString()).append(".cmrep").c_str());
#elif __linux__
	cmrepFile.open((fileName.toStdString()).append(".cmrep").c_str());
#endif

	cmrepFile<<"Grid.Type = ";
	if(this->GridTypeComboBox->currentIndex() == 0){
		cmrepFile << "LoopSubdivision";
	}
	cmrepFile << std::endl;

	cmrepFile<<"Grid.Model.SolverType = ";
	if(this->SolverTypeComboBox->currentIndex() == 0)
		cmrepFile<<"BruteForce";
	else if(this->SolverTypeComboBox->currentIndex() == 1)
		cmrepFile<<"PDE";
	cmrepFile << std::endl;

	if(this->GridTypeComboBox->currentIndex() == 0){
		cmrepFile<<"Grid.Model.Atom.SubdivisionLevel = ";
		switch(this->SubLevelComboBox->currentIndex()){
		case 0: 
			cmrepFile<<"0";
			break;
		case 1: 
			cmrepFile<<"1";
			break;
		case 2:
			cmrepFile<<"2";
			break;
		case 3:
			cmrepFile<<"3";
			break;
		}
	}
	cmrepFile << std::endl;

	cmrepFile<<"Grid.Model.Coefficient.FileName = ";
	std::string name = (fileName.toStdString()).append(".vtk");
	int lastSlash;

#ifdef _WIN64
	lastSlash = name.find_last_of("/");
#elif _WIN32
	lastSlash = name.find_last_of("/");
#elif __APPLE__
	lastSlash = name.find_last_of("/");
#elif __linux__
	lastSlash = name.find_last_of("\\");
#endif
	
	cmrepFile<<name.substr(lastSlash+1, name.size());
	cmrepFile << std::endl;

	cmrepFile<<"Grid.Model.Coefficient.FileType = VTK";
	cmrepFile << std::endl;

	if(this->SolverTypeComboBox->currentIndex() == 1){
		cmrepFile<<"Grid.Model.Coefficient.ConstantRho = ";
		cmrepFile<<this->RhoLineEdit->text().toStdString();
		cmrepFile << std::endl;
	}

	if(this->ConsRadiusCheckBox->isChecked()){
		cmrepFile<<"Grid.Model.Coefficient.ConstantRadius = ";
		cmrepFile<<this->RadiusLineEdit->text().toStdString();
		cmrepFile << std::endl;
	}

	cmrepFile<<"Grid.Model.nLabels = ";
	std::vector<bool>trackNumLabel;
	trackNumLabel.resize(10);
	for(int i = 0; i < Global::vectorTagInfo.size(); i ++)
	{
		trackNumLabel[Global::vectorTagInfo[i].tagIndex] = true;
	}

	int numCount = 0;
	for(int i = 0; i < trackNumLabel.size(); i++)
		if(trackNumLabel[i])
			numCount ++;
	cmrepFile<<numCount;
	cmrepFile << std::endl;

	cmrepFile.close();
	
}

void EventQtSlotConnect::loadSettings()
{
	std::cout << settingsFile.toStdString() << std::endl;
	QSettings settings(settingsFile, QSettings::IniFormat);
	QString pText = settings.value("path", "").toString();
	this->pathQvoronoi->setText(pText);
}

void EventQtSlotConnect::saveSettings()
{
	QSettings settings(settingsFile, QSettings::IniFormat);
	QString pText = this->pathQvoronoi->text();
	settings.setValue("path", pText);
}

//from pyushkevich/cmrep/src/ConstrainedCMRepTest.cxx
template<class T>
unsigned int count_nnz(vnl_sparse_matrix<T> &mat)
{
	unsigned int nnz = 0;
	for (unsigned int i = 0; i < mat.rows(); i++)
	{
		auto &r = mat.get_row(i);
		for (unsigned int j = 0; j < r.size(); j++)
		{
			if (r[j].second != 0.0)
				nnz++;
		}
	}
	return nnz;
}

//void EventQtSlotConnect::InflateMedialModelWithBranches(const char *fn_input, const char *fn_output, double rad, int edge_label)
void EventQtSlotConnect::InflateMedialModelWithBranches(QString fn_output, double rad)
{
	// This inflation code accepts non-mesh medial surfaces, i.e., medial surfaces with branches. We can no
	// longer use the half-edge construct.
	fn_output.replace(".vtk", "");

	// An edge is a pair of vertices, always stored in sorted order
	typedef std::pair<unsigned int, unsigned int> Edge;

	// A reference to a triangle edge (tri index, edge index, forward/backward)
	typedef std::tuple<unsigned int, unsigned int, bool> TriEdgeRef;

	// Each edge is associated with some number of triangles
	typedef std::map<Edge, std::list<TriEdgeRef> > EdgeTriMap;

	// Edge-triangle map
	EdgeTriMap etm;

	// List of duplicate triangles
	std::vector<Triangle> tdup;

	// List of triangle normals
	typedef vnl_vector_fixed<double, 3> Vec3;
	std::vector<Vec3> tnorm;

	// Find all the edges in the mesh
	// for (unsigned int i = 0; i < pd->GetNumberOfCells(); i++)
	for (unsigned int i = 0; i < Global::vectorTagTriangles.size(); i++)
	{
		// Create duplicates of the triangle with opposite windings
		Triangle t1;
		t1.vertices[0] = Global::vectorTagTriangles[i].id1;
		t1.vertices[1] = Global::vectorTagTriangles[i].id2;
		t1.vertices[2] = Global::vectorTagTriangles[i].id3;
		tdup.push_back(t1);

		Triangle t2;
		t2.vertices[0] = Global::vectorTagTriangles[i].id3;
		t2.vertices[1] = Global::vectorTagTriangles[i].id2;
		t2.vertices[2] = Global::vectorTagTriangles[i].id1;
		tdup.push_back(t2);

		// Compute the normals of the two triangles
		Vec3 A(Global::vectorTagTriangles[i].p1);
		Vec3 B(Global::vectorTagTriangles[i].p2);
		Vec3 C(Global::vectorTagTriangles[i].p3);
		Vec3 N = vnl_cross_3d(B - A, C - A).normalize();

		tnorm.push_back(N);
		tnorm.push_back(-N);
	}

	// Find edges across the duplicate triangles
	for (unsigned int i = 0; i < tdup.size(); i++)
	{
		for (unsigned int k = 0; k < 3; k++)
		{
			size_t v1 = tdup[i].vertices[(k + 1) % 3]; //1>2>0
			size_t v2 = tdup[i].vertices[(k + 2) % 3]; //2>0>1
			Edge ek = make_pair(min(v1, v2), max(v1, v2));

			// Add the triangle to this edge, marking it as either forward-traversed
			// or backward traversed.
			etm[ek].push_back(std::make_tuple(i, k, v1 > v2)); //create a tuple 
		}
	}

	// For each edge, find the triangles that are adjacent across the edge. Adjacent
	// triangles must traverse the edge in opposite order.
	for (auto &eit : etm)
	{
		// Get the edge vector direction
		Vec3 e_X1(Global::vectorTagPoints[eit.first.first].pos);
		Vec3 e_X2(Global::vectorTagPoints[eit.first.second].pos);

		for (auto &tref : eit.second)
		{
			// Get the normal of the current triangle
			unsigned int i_tri = std::get<0>(tref);
			unsigned int i_tri_edge_idx = std::get<1>(tref);
			bool winding = std::get<2>(tref);

			const Vec3 &N = tnorm[i_tri];
			Vec3 Z = (e_X2 - e_X1).normalize();
			if (!winding)
				Z = -Z;
			Vec3 X = vnl_cross_3d(Z, N);

			// Find the triangle that is closest by converting each opposite-winded triangle
			// to an angle and selecting the one with the minimum angle
			unsigned int opp_tri = NOID, opp_tri_edge_idx = -1;
			double min_angle = 0.0;
			for (auto &tref_test : eit.second)
			{
				// Only consider opposite winding
				if (std::get<2>(tref_test) != std::get<2>(tref))
				{
					// Find the 'X' of the test triangle
					unsigned int i_tri_test = std::get<0>(tref_test);
					const Vec3 &N_test = -tnorm[i_tri_test];
					Vec3 X_test = vnl_cross_3d(Z, N_test);

					// Find the angle with the current triangle.
					double a_test = (i_tri / 2 == i_tri_test / 2)
						? vnl_math::twopi
						: atan2(dot_product(X_test, N), dot_product(X_test, X));
					if (a_test <= 0.0)
						a_test += vnl_math::twopi;

					printf("Angle of triangle %d with triangle %d over edge (%d,%d) is %f\n",
						i_tri, i_tri_test, eit.first.first, eit.first.second, a_test);

					// Is this the best match
					if (opp_tri == NOID || a_test < min_angle)
					{
						opp_tri = i_tri_test;
						opp_tri_edge_idx = std::get<1>(tref_test);
						min_angle = a_test;
					}
				}
			}

			// We can now mark the neighbor of the triangle across this edge
			tdup[i_tri].neighbors[i_tri_edge_idx] = opp_tri;
			tdup[i_tri].nedges[i_tri_edge_idx] = (short)opp_tri_edge_idx;

			printf("Triangle %d matched to triangle %d\n", i_tri, opp_tri);
		}
	}

	// Create a vertex adjacency matrix. The rows/columns refer to the triangle vertices
	// which are at this point all considered to be disjoint points. When the adjacency
	// matrix contains 1, this means that the two vertices are actually the same point
	vnl_sparse_matrix<int> tv_adj(tdup.size() * 3, tdup.size() * 3);

	// Visit each edge in each triangle and match the vertices with the opposite edge
	// in the opposite triangle
	for (unsigned int i = 0; i < tdup.size(); i++)
	{
		for (unsigned int k = 0; k < 3; k++)
		{
			// Add identity element to matrix
			tv_adj(i * 3 + k, i * 3 + k) = 1;

			// Take triangle that's opposite
			unsigned int i_opp = tdup[i].neighbors[k];
			if (i_opp == NOID)
				std::cout << "Exception: Triangle missing neighbor" << std::endl;

			// Set the matches
			unsigned int k_opp = tdup[i].nedges[k];
			unsigned int v1 = (k + 1) % 3, v2 = (k + 2) % 3;
			unsigned int v1_opp = (k_opp + 1) % 3, v2_opp = (k_opp + 2) % 3;

			tv_adj(i * 3 + v1, i_opp * 3 + v2_opp) = 1;
			tv_adj(i * 3 + v2, i_opp * 3 + v1_opp) = 1;
		}
	}

	// Find the connected components in the adjacency matrix. A lazy way to do this is to take powers of the
	// matrix until it converges.
	unsigned int nnz_last = count_nnz(tv_adj);
	printf("Adjacency matrix, nnz = %d\n", nnz_last);
	vnl_sparse_matrix<int> tv_adj_pow = tv_adj * tv_adj;
	while (count_nnz(tv_adj_pow) > nnz_last)
	{
		nnz_last = count_nnz(tv_adj_pow);
		tv_adj_pow = tv_adj_pow * tv_adj;
		printf("Adjacency multiplication, nnz = %d\n", nnz_last);
	}

	// Go through and remap the disjoint vertices to new vertices
	std::vector<unsigned int> vnew(tdup.size() * 3, NOID);
	unsigned int vcurr = 0;
	for (unsigned int i = 0; i < tdup.size() * 3; i++)
	{
		if (vnew[i] == NOID)
		{
			// Assign a new vertex ID to this vertex
			vnew[i] = vcurr;

			// Assign it to every other vertex in its row
			auto &row = tv_adj_pow.get_row(i);
			for (unsigned int j = 0; j < row.size(); j++)
			{
				if (vnew[row[j].first] != NOID && vnew[row[j].first] != vcurr)
					std::cout << "Exception: Vertex traversal logic violation" << std::endl;

				vnew[row[j].first] = vcurr;
			}
			vcurr++;
		}
	} 
	// Now we have a valid mesh structure in place. We can store this into a proper
	// triangle array
	vnl_matrix<unsigned int> m_tri(tdup.size(), 3);

	// We also need to compute the positions of the new vertices, i.e., by pushing them out
	// along the outward normals. We initialize each point to its original mesh location and
	// then add to the vertex all the normals of all the triangles that contain it
	vnl_matrix<double> m_pt(vcurr, 3), m_pt_offset(vcurr, 3);
	std::vector<unsigned int> valence(vcurr, 0);

	// Create the medial index array - this is just the original medial vertex
	vnl_vector<int> m_mindex(vcurr);

	// First pass through triangles, assigning new vertices and vertex coordinates
	for (unsigned int i = 0; i < tdup.size(); i++)
	{
		// Compute the triangle normal and center
		Vec3 P[] = {
			Vec3(Global::vectorTagPoints[tdup[i].vertices[0]].pos),
			Vec3(Global::vectorTagPoints[tdup[i].vertices[1]].pos),
			Vec3(Global::vectorTagPoints[tdup[i].vertices[2]].pos) };

		Vec3 N = vnl_cross_3d(P[1] - P[0], P[2] - P[0]).normalize();
		Vec3 C = (P[0] + P[1] + P[2]) / 3.0;

		for (unsigned int k = 0; k < 3; k++)
		{
			// Assign the new vertex
			m_tri(i, k) = vnew[i * 3 + k];

			// Get the coordinate of this vertex
			m_pt.set_row(m_tri(i, k), P[k]);

			// Add up the valence of this vertex
			valence[m_tri(i, k)]++;

			// Add up to the shift vector
			m_pt_offset.set_row(m_tri(i, k), m_pt_offset.get_row(m_tri(i, k)) + N);

			// Set the medial index (original index before inflation)
			m_mindex[m_tri(i, k)] = tdup[i].vertices[k];
		}
	}
	// Offset the vertices
	for (unsigned int j = 0; j < vcurr; j++)
		m_pt.set_row(j, m_pt.get_row(j) + rad * m_pt_offset.get_row(j) / valence[j]);
	
	vtkSmartPointer<vtkPolyData> vmb = vtkPolyData::New();

	vtkSmartPointer<vtkCellArray> cells = vtkCellArray::New();
	for (unsigned int i = 0; i < m_tri.rows(); i++)
	{
		cells->InsertNextCell(3);
		for (unsigned int a = 0; a < 3; a++)
			cells->InsertCellPoint(m_tri(i, a));
	}
	vmb->SetPolys(cells);
	assert(m_pt.columns() == 3);
	vtkSmartPointer<vtkPoints> pts = vtkPoints::New();
	pts->SetNumberOfPoints(m_pt.rows());
	for (int i = 0; i < m_pt.rows(); i++)
		pts->SetPoint(i, m_pt(i, 0), m_pt(i, 1), m_pt(i, 2));
	
	vmb->SetPoints(pts);

#ifdef _WIN64 
	assert(Global::vectorTagPoints.size() == m_mindex.size());
#elif _WIN32
	assert(Global::vectorTagPoints.size() == m_mindex.size());
#endif
	vtkSmartPointer<vtkIntArray> arr = vtkIntArray::New();
	arr->SetNumberOfComponents(1);
	arr->SetNumberOfTuples(m_mindex.size());
	arr->SetName("MedialIndex");
	// Update the points
	for (int i = 0; i < m_mindex.size(); i++)
		arr->SetTuple1(i, m_mindex[i]);
	vmb->GetPointData()->AddArray(arr);
	vtkSmartPointer<vtkPolyDataWriter> w = vtkPolyDataWriter::New();
	w->SetInputData(vmb);
	w->SetFileName(fn_output.toStdString().append("_inflate.vtk").c_str());
	w->Update();
}

//Triangle label initialization
/*void EventQtSlotConnect::iniTriLabel()
{
	for(int i = 0; i < 10; i++)
	{
		QPixmap pix(22,22);
		QString displayText = QString::number(i + 1);
		QColor qc = QColor::fromRgb(rand() % 255, rand() % 255, rand() % 255);
		triLabelColors[i] = qc;
		hideTriLabel[i] = 0;
		mouseInteractor->triLabelColors[i] = qc;
		if(i == 0){
			Global::triCol[0] = qc.red() / 255.0;
			Global::triCol[1] = qc.green() / 255.0;
			Global::triCol[2] = qc.blue() / 255.0;
			mouseInteractor->currentTriIndex = i;
		}
		pix.fill(qc);
		this->TriLabelComboBox->addItem(pix, displayText);
	}
}*/
