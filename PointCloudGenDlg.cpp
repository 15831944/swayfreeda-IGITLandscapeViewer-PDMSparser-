#include"PointCloudGenDlg.h"
#include"PointCloudGenProgressDlg.h"

#include<qsettings.h>
#include "BinFilter.h"
#include "ccPersistentSettings.h"
#include "ccProgressDialog.h"
#include <qfiledialog.h>
#include <qfile.h>
#include <qtextstream.h>
#include <qimage.h>
#include <qmessagebox.h>
#include <qprocess.h>
#include <qprogressdialog.h>
#include <qmainwindow.h>
#include <qdir.h>
#include<Shellapi.h>
//#include <tchar.h>

static const QString s_allImgFilesFilter("Images(*.JPG *.jpg *.png *.PNG)");

//default file filter separator
static const QString s_fileFilterSeparator(";;");


//================================================pointCloudGenDlg========================================================//
PointCloudGenDlg::PointCloudGenDlg(QWidget *parent){
	
	setupUi(this);
	pushButtonStartReconstruction->setEnabled(false);
	m_outFolderDir = QDir::currentPath();
	setWindowFlags(Qt::Dialog | Qt::WindowMaximizeButtonHint); //��󻯰�ť
	connect(pushButtonLoadImgs,                 SIGNAL(clicked()),                            this,        SLOT(doActionLoadImageFiles()));
	connect(pushButtonStartReconstruction,      SIGNAL(clicked()),                          this,          SLOT(startReconstruction()));
}

//=================================================doActionLoadImageFiles==================================================//
void PointCloudGenDlg::doActionLoadImageFiles(){

	QSettings settings;
	settings.beginGroup(ccPS::LoadFile());
	QString currentPath = settings.value(ccPS::CurrentPath(),QApplication::applicationDirPath()).toString();
	QString currentOpenDlgFilter = settings.value(ccPS::SelectedInputFilter(), BinFilter::GetFileFilter()).toString();

		// Add all available file I/O filters (with import capabilities)
	QStringList fileFilters;
	fileFilters<<s_allImgFilesFilter;
	//currentOpenDlgFilter = s_allImgFilesFilter;

	//file choosing dialog
	QStringList selectedFiles = QFileDialog::getOpenFileNames(	this,
																"��ͼ���ļ�",
																currentPath,
																fileFilters.join(s_fileFilterSeparator),
																&currentOpenDlgFilter
#ifdef _DEBUG
																,QFileDialog::DontUseNativeDialog
#endif
															);
	if (selectedFiles.isEmpty()){
		QMessageBox::warning(this, "Warning", "No Image File Existed!");
		return;
	}

	QDir dir;
	if(QFile::exists("imgList.txt")) QFile::remove("imgList.txt");
	QFile file("imgList.txt");
	if(!file.open(QIODevice::WriteOnly)){
		QMessageBox::warning(this, "Warning", "Failed to Create File [imageLisT.txt],\n Make sure the administrative privileges are given!");
		return;
	}
	QTextStream out(&file);
	
	QImage img;
	int valid_img_num=0;
	int invalid_img_num=0;

	QVector<QString> imgDirs;
	foreach(QString path, selectedFiles){
		out<<path<<endl;
		imgDirs.append(path);

		if(1/*img.load(path)*/) {valid_img_num++;}
		else invalid_img_num++;
	}
	createImgList(imgDirs);

	QString text;
	text = QString("��Чͼ����Ŀ�� %1\n").arg(valid_img_num);
	text += QString("��Чͼ�����: %1\n").arg(invalid_img_num);
	//QMessageBox::about(this, "����ͼ����Ϣ", text);
	QMessageBox message(QMessageBox::NoIcon, "����ͼ����Ϣ", text, QMessageBox::Yes | QMessageBox::No, NULL); 
    if(message.exec() == QMessageBox::No){ 
		return;
	}
	if(valid_img_num>0){
		pushButtonStartReconstruction->setEnabled(true);
	}

	//save last loading parameters
	currentPath = QFileInfo(selectedFiles[0]).absolutePath();
	settings.setValue(ccPS::CurrentPath(),currentPath);
	settings.setValue(ccPS::SelectedInputFilter(),currentOpenDlgFilter);
	settings.endGroup();

	update();
}

void PointCloudGenDlg::createImgList(QVector<QString>&imgDirs){

	int W_ICONSIZE = 120;
	int H_ICONSIZE = 90;
	//����QListWidget�еĵ�Ԫ���ͼƬ��С
      listWidgetImgList->setIconSize(QSize(W_ICONSIZE,H_ICONSIZE));
      listWidgetImgList->setResizeMode(QListView::Adjust);
     //����QListWidget����ʾģʽ
     listWidgetImgList->setViewMode(QListView::IconMode);
     //����QListWidget�еĵ�Ԫ��ɱ��϶�
     listWidgetImgList->setMovement(QListView::Static);
     //����QListWidget�еĵ�Ԫ��ļ��
     listWidgetImgList->setSpacing(10);

	QProgressDialog dialog;
	dialog.setLabelText("Loading Images ...");
	dialog.setRange(0, imgDirs.size());

	 int id = 0;
	 foreach(QString path, imgDirs){
		 QPixmap objPixmap(path);		 

		 QStringList words = path.split("/");
		 QString imgName = words.takeLast();
		
		 // ����QListWidgetItem����(ע�⣺��Iconͼ�����������[96*96])---scaled����
		 QListWidgetItem *pItem = new QListWidgetItem(QIcon(objPixmap.scaled(QSize(W_ICONSIZE,H_ICONSIZE))),imgName);
		
		 //���õ�Ԫ��Ŀ�Ⱥ͸߶�
        pItem->setSizeHint(QSize(W_ICONSIZE,H_ICONSIZE+15));
        listWidgetImgList->insertItem(id,pItem);

		dialog.setValue(id);
		qApp->processEvents();
		if(dialog.wasCanceled()){
			return;
		}
		update();
		id++;
	 }

	 listWidgetImgList->setMouseTracking(true);
	 //�����źŲ�
     connect(listWidgetImgList,SIGNAL(itemClicked(QListWidgetItem*)),this,SLOT(process(QListWidgetItem*)));
	 update();
}

//=================================================startReconstruction======================================================//
void PointCloudGenDlg::startReconstruction(){

	PointCloudGenProgressDlg dialog;
	if(!dialog.exec()){
	  m_outFolderDir = dialog.getFullOutPath();
	  this->close();
	}
			

}
//=================================================process=================================================================//
void PointCloudGenDlg::process(QListWidgetItem* item){
	//item->setBackgroundColor(Qt::blue);
	update();
}

