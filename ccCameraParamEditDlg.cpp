//##########################################################################
//#                                                                        #
//#                            CLOUDCOMPARE                                #
//#                                                                        #
//#  This program is free software; you can redistribute it and/or modify  #
//#  it under the terms of the GNU General Public License as published by  #
//#  the Free Software Foundation; version 2 of the License.               #
//#                                                                        #
//#  This program is distributed in the hope that it will be useful,       #
//#  but WITHOUT ANY WARRANTY; without even the implied warranty of        #
//#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         #
//#  GNU General Public License for more details.                          #
//#                                                                        #
//#          COPYRIGHT: EDF R&D / TELECOM ParisTech (ENST-TSI)             #
//#                                                                        #
//##########################################################################

#include "ccCameraParamEditDlg.h"

//Local
#include "ccPointCloud.h"

//qCC_db
#include <ccGLUtils.h>
#include <ccHObjectCaster.h>
#include <ccGenericMesh.h>

//CCLib
#include <CCConst.h>

//Qt
#include <QDoubleValidator>
#include <QMdiSubWindow>

// convert value from slider 
double SliderPosToZNearCoef(int i, int iMax)
{
	assert(i >= 0 && i <= iMax);
	return pow(10,-static_cast<double>((iMax-i)*3)/iMax); //between 10^-3 and 1
}
//convert value for Slider
int ZNearCoefToSliderPos(double coef, int iMax)
{
	assert(coef >= 0 && coef <= 1.0);
	int i = static_cast<int>(-(static_cast<double>(iMax)/3) * log10(coef));
	assert(i >= 0 && i <= iMax);
	return iMax-i;
}

ccCameraParamEditDlg::ccCameraParamEditDlg(QWidget* parent)
	: ccOverlayDialog(parent)
	, Ui::CameraParamDlg()
{
	setupUi(this);

	//Slider�е�ֵ�仯����SpinBox��ֵҲ��֮�����仯��ͬʱ����͸�Ӿ���
	connect(phiSlider,				SIGNAL(valueChanged(int)),		this,	SLOT(iPhiValueChanged(int)));
	connect(thetaSlider,			SIGNAL(valueChanged(int)),		this,	SLOT(iThetaValueChanged(int)));
	connect(psiSlider,				SIGNAL(valueChanged(int)),		this,	SLOT(iPsiValueChanged(int)));

	//SpinBox�е�ֵ�����仯����Slider��ֵҲ��֮�����仯��ͬʱ����͸�Ӿ���
	connect(phiSpinBox,				SIGNAL(valueChanged(double)),	this,	SLOT(dPhiValueChanged(double)));
	connect(thetaSpinBox,			SIGNAL(valueChanged(double)),	this,	SLOT(dThetaValueChanged(double)));
	connect(psiSpinBox,				SIGNAL(valueChanged(double)),	this,	SLOT(dPsiValueChanged(double)));

	//rotation center//��ת���ķ����仯
	connect(rcxDoubleSpinBox,		SIGNAL(valueChanged(double)),	this,	SLOT(pivotChanged()));
	connect(rcyDoubleSpinBox,		SIGNAL(valueChanged(double)),	this,	SLOT(pivotChanged()));
	connect(rczDoubleSpinBox,		SIGNAL(valueChanged(double)),	this,	SLOT(pivotChanged()));

	//camera center//������ķ����仯
	connect(exDoubleSpinBox,		SIGNAL(valueChanged(double)),	this,	SLOT(cameraCenterChanged()));
	connect(eyDoubleSpinBox,		SIGNAL(valueChanged(double)),	this,	SLOT(cameraCenterChanged()));
	connect(ezDoubleSpinBox,		SIGNAL(valueChanged(double)),	this,	SLOT(cameraCenterChanged()));

	//�ӽǱ仯
	connect(fovDoubleSpinBox,		SIGNAL(valueChanged(double)),	this,	SLOT(fovChanged(double)));

	//�����ƽ�淢���仯
	connect(zNearHorizontalSlider,	SIGNAL(sliderMoved(int)),		this,	SLOT(zNearSliderMoved(int)));

	//�����ӽ�
	connect(viewUpToolButton,		SIGNAL(clicked()),				this,	SLOT(setTopView()));
	connect(viewDownToolButton,		SIGNAL(clicked()),				this,	SLOT(setBottomView()));
	connect(viewFrontToolButton,	SIGNAL(clicked()),				this,	SLOT(setFrontView()));
	connect(viewBackToolButton,		SIGNAL(clicked()),				this,	SLOT(setBackView()));
	connect(viewLeftToolButton,		SIGNAL(clicked()),				this,	SLOT(setLeftView()));
	connect(viewRightToolButton,	SIGNAL(clicked()),				this,	SLOT(setRightView()));
	connect(viewIso1ToolButton,		SIGNAL(clicked()),				this,	SLOT(setIso1View()));
	connect(viewIso2ToolButton,		SIGNAL(clicked()),				this,	SLOT(setIso2View()));

	//���浱ǰ����
	connect(pushMatrixToolButton,	SIGNAL(clicked()),				this,	SLOT(pushCurrentMatrix()));
	connect(revertMatrixToolButton,	SIGNAL(clicked()),				this,	SLOT(revertToPushedMatrix()));

	//ѡ�����Ϊ��ת������
	connect(pivotPickingToolButton,	SIGNAL(clicked()),				this,	SLOT(pickPointAsPivot()));
}

ccCameraParamEditDlg::~ccCameraParamEditDlg()
{
}

void ccCameraParamEditDlg::makeFrameless()
{
	setWindowFlags(Qt::FramelessWindowHint |Qt::Tool);
}

void ccCameraParamEditDlg::iThetaValueChanged(int val)
{
	thetaSpinBox->blockSignals(true);
	thetaSpinBox->setValue(static_cast<double>(val)/10);
	thetaSpinBox->blockSignals(false);

	reflectParamChange();
}

void ccCameraParamEditDlg::iPsiValueChanged(int val)
{
	psiSpinBox->blockSignals(true);
	psiSpinBox->setValue(static_cast<double>(val)/10);
	psiSpinBox->blockSignals(false);

	reflectParamChange();
}

void ccCameraParamEditDlg::iPhiValueChanged(int val)
{
	phiSpinBox->blockSignals(true);
	phiSpinBox->setValue(static_cast<double>(val)/10);
	phiSpinBox->blockSignals(false);

	//�ı�͸�Ӿ���
	reflectParamChange();
}

void ccCameraParamEditDlg::dThetaValueChanged(double val)
{
	thetaSlider->blockSignals(true);
	thetaSlider->setValue(static_cast<int>(val*10.0));
	thetaSlider->blockSignals(false);
	reflectParamChange();
}

void ccCameraParamEditDlg::dPsiValueChanged(double val)
{
	psiSlider->blockSignals(true);
	psiSlider->setValue(static_cast<int>(val*10.0));
	psiSlider->blockSignals(false);
	reflectParamChange();
}

void ccCameraParamEditDlg::dPhiValueChanged(double val)
{
	phiSlider->blockSignals(true);
	phiSlider->setValue(static_cast<int>(val*10.0));
	phiSlider->blockSignals(false);

	reflectParamChange();
}
//=============================================cameraCenterChanged=========================================//
// GLWindow->setCameraPos(CCVector3d & P);
void ccCameraParamEditDlg::cameraCenterChanged()
{
	if (!m_associatedWin)
		return;

	m_associatedWin->blockSignals(true);
	m_associatedWin->setCameraPos( CCVector3d(	exDoubleSpinBox->value(),
												eyDoubleSpinBox->value(),
												ezDoubleSpinBox->value() ));
	m_associatedWin->blockSignals(false);

	m_associatedWin->redraw();
}

//============================================pivotChanged==================================================//
// GLWindow->setPivotPoint(CCVector3d & P);//���õ�ʱ�������ź�
void ccCameraParamEditDlg::pivotChanged()
{
	if (!m_associatedWin)	return;

	m_associatedWin->blockSignals(true);
	m_associatedWin->setPivotPoint(CCVector3d(rcxDoubleSpinBox->value(),
		                                      rcyDoubleSpinBox->value(),
		                                      rczDoubleSpinBox->value()) );
	m_associatedWin->blockSignals(false);
	
	//���»��Ƴ���
	m_associatedWin->redraw();
}
//========================================================fovChaged=========================================//
//GLWindow->setFov(float angle)
void ccCameraParamEditDlg::fovChanged(double value)
{
	if (!m_associatedWin)return;

	m_associatedWin->blockSignals(true);
	m_associatedWin->setFov(static_cast<float>(value));
	m_associatedWin->blockSignals(false);

	m_associatedWin->redraw();
}
//=====================================================zNearSliderMoved=====================================//
void ccCameraParamEditDlg::zNearSliderMoved(int i)
{
	if (!m_associatedWin)return;

	double zNearCoef = SliderPosToZNearCoef(i,zNearHorizontalSlider->maximum());
	m_associatedWin->blockSignals(true);
	m_associatedWin->setZNearCoef(zNearCoef);
	m_associatedWin->blockSignals(false);

	m_associatedWin->redraw();
}
//====================================================pushCurrentMatrix====================================//
void ccCameraParamEditDlg::pushCurrentMatrix()
{
	if (!m_associatedWin)return;
	//��ȡ͸�Ӿ���
	ccGLMatrixd mat = m_associatedWin->getBaseViewMat();

	std::pair<PushedMatricesMapType::iterator,bool> ret;
	ret = pushedMatrices.insert(PushedMatricesMapElement(m_associatedWin,mat));
	if (ret.second == false) //already exists //��������Ѿ������ڣ�����¾���
		ret.first->second = mat;

	//ֻ�д洢����֮����ܿ�����ع���
	buttonsFrame->setEnabled(true);
}
//============================================revertToPushMatrix==============================================//
void ccCameraParamEditDlg::revertToPushedMatrix()
{
	//�ָ���ǰ����
	PushedMatricesMapType::iterator it = pushedMatrices.find(m_associatedWin);
	if (it == pushedMatrices.end())
		return;

	initWithMatrix(it->second);
	m_associatedWin->blockSignals(true);
	m_associatedWin->setBaseViewMat(it->second);
	m_associatedWin->blockSignals(false);
	m_associatedWin->redraw();
}

void ccCameraParamEditDlg::pickPointAsPivot()
{
	if (m_associatedWin)
	{
		m_associatedWin->setPickingMode(ccGLWindow::POINT_OR_TRIANGLE_PICKING);
		connect(m_associatedWin, SIGNAL(itemPicked(int, unsigned, int, int)), this, SLOT(processPickedItem(int, unsigned, int, int)));
	}
}

void ccCameraParamEditDlg::processPickedItem(int entityID, unsigned itemIndex, int x, int y)
{
	if (!m_associatedWin)
		return;

	ccHObject* obj = 0;
	ccHObject* db = m_associatedWin->getSceneDB();
	if (db)
		obj = db->find(entityID);
	if (obj)
	{
		CCVector3 P;
		if (obj->isKindOf(CC_TYPES::POINT_CLOUD))
		{
			ccGenericPointCloud* cloud = ccHObjectCaster::ToGenericPointCloud(obj);
			if (!cloud)
			{
				assert(false);
				return;
			}
			P = *cloud->getPoint(itemIndex);
		}
		else if (obj->isKindOf(CC_TYPES::MESH))
		{
			ccGenericMesh* mesh = ccHObjectCaster::ToGenericMesh(obj);
			if (!mesh)
			{
				assert(false);
				return;
			}
			CCLib::GenericTriangle* tri = mesh->_getTriangle(itemIndex);
			P = m_associatedWin->backprojectPointOnTriangle(CCVector2i(x,y),*tri->_getA(),*tri->_getB(),*tri->_getC());
		}
		else
		{
			//unhandled entity
			assert(false);
			return;
		}

		m_associatedWin->setPivotPoint(CCVector3d::fromArray(P.u));
		m_associatedWin->redraw();
	}

	m_associatedWin->setPickingMode(ccGLWindow::DEFAULT_PICKING);
	disconnect(m_associatedWin, SIGNAL(itemPicked(int, unsigned, int, int)), this, SLOT(processPickedItem(int, unsigned, int, int)));
}

//===================================================setView==============================================//
void ccCameraParamEditDlg::setView(CC_VIEW_ORIENTATION orientation)
{
	if (!m_associatedWin)return;

	//��ȡ�뵱ǰ������صľ���
	PushedMatricesMapType::iterator it = pushedMatrices.find(m_associatedWin);

	//??????????????????????????????????????????
	m_associatedWin->makeCurrent();

	//���ӽǵľ��������ڵ�ǰ����
	ccGLMatrixd mat = ccGLUtils::GenerateViewMat(orientation) * (it->second);
	//ccGLMatrixd mat = ccGLUtils::GenerateViewMat(orientation);

	//ͨ������õ������Ƕ�ֵ���ı�slider �� spinBox��ֵ
	initWithMatrix(mat);

	//�����������ڴ���
	m_associatedWin->blockSignals(true);
	m_associatedWin->setBaseViewMat(mat);
	m_associatedWin->blockSignals(false);
	m_associatedWin->redraw();
}
//====================================================setTopView==========================================//
void ccCameraParamEditDlg::setTopView(){
	setView(CC_TOP_VIEW);
}
//====================================================setBottonView=======================================//
void ccCameraParamEditDlg::setBottomView()
{
	setView(CC_BOTTOM_VIEW);
}
//===================================================setFrontView==========================================//
void ccCameraParamEditDlg::setFrontView()
{
	setView(CC_FRONT_VIEW);
}
//===================================================setBackView===========================================//
void ccCameraParamEditDlg::setBackView()
{
	setView(CC_BACK_VIEW);
}
//====================================================setLeftView========================================//
void ccCameraParamEditDlg::setLeftView()
{
	setView(CC_LEFT_VIEW);
}
///===================================================setRgihtView===========================================//
void ccCameraParamEditDlg::setRightView()
{
	setView(CC_RIGHT_VIEW);
}
//====================================================setIso1View=========================================//
void ccCameraParamEditDlg::setIso1View()
{
	setView(CC_ISO_VIEW_1);
}
//=====================================================setIso2View=============================================//
void ccCameraParamEditDlg::setIso2View()
{
	setView(CC_ISO_VIEW_2);
}

//======================================================start================================================//
//��ʼ��ʾ
bool ccCameraParamEditDlg::start()
{
	ccOverlayDialog::start();

	m_processing = false; //no such concept for this dialog! (+ we want to allow dynamic change of associated window)

	return true;
}

//=====================================================linkWith===============================================//
void ccCameraParamEditDlg::linkWith(QMdiSubWindow* qWin)
{
	//corresponding ccGLWindow
	ccGLWindow* associatedWin = (qWin ? static_cast<ccGLWindow*>(qWin->widget()) : 0);

	linkWith(associatedWin);
}
//=====================================================linkWiht================================================//
bool ccCameraParamEditDlg::linkWith(ccGLWindow* win)
{
	ccGLWindow* oldWin = m_associatedWin;//���m_associatedWin==0 ��˵��û�б����ӹ�

	if (!ccOverlayDialog::linkWith(win))return false;//�ڴ�����m_associatedWin��ֵ

	if (oldWin){
		m_associatedWin->disconnect(this);
	}

	if (m_associatedWin)
	{
		initWith(m_associatedWin);
		connect(m_associatedWin,	SIGNAL(baseViewMatChanged(const ccGLMatrixd&)),		this,	SLOT(initWithMatrix(const ccGLMatrixd&)));
		connect(m_associatedWin,	SIGNAL(viewMatRotated(const ccGLMatrixd&)),			this,	SLOT(updateViewMatrix(const ccGLMatrixd&)));

		connect(m_associatedWin,	SIGNAL(cameraPosChanged(const CCVector3d&)),		this,	SLOT(updateCameraCenter(const CCVector3d&)));
		connect(m_associatedWin,	SIGNAL(pivotPointChanged(const CCVector3d&)),		this,	SLOT(updatePivotPoint(const CCVector3d&)));
		connect(m_associatedWin,	SIGNAL(perspectiveStateChanged()),					this,	SLOT(updateViewMode()));
		connect(m_associatedWin,	SIGNAL(destroyed(QObject*)),						this,	SLOT(hide()));
		connect(m_associatedWin,	SIGNAL(fovChanged(float)),							this,	SLOT(updateWinFov(float)));

		PushedMatricesMapType::iterator it = pushedMatrices.find(m_associatedWin);
		buttonsFrame->setEnabled(it != pushedMatrices.end());//disabled the parameters//��ʼʱ�ڴ˽���
	}
	else
	{
		hide();
		buttonsFrame->setEnabled(false);
	}

	return true;
}

//==============================================reflectParamChange==============================================//
void ccCameraParamEditDlg::reflectParamChange()
{
	if (!m_associatedWin)
		return;

	ccGLMatrixd mat = getMatrix();
	m_associatedWin->blockSignals(true);
	m_associatedWin->setBaseViewMat(mat);
	m_associatedWin->blockSignals(false);
	//���»���
	m_associatedWin->redraw();
}
//============================================updateViewMode======================================================//
void ccCameraParamEditDlg::updateViewMode()
{
	if (m_associatedWin){
		bool objectBased = true;
		bool perspective = m_associatedWin->getPerspectiveState(objectBased);

		if (!perspective)
			//currentModeLabel->setText("parallel projection");
			currentModeLabel->setText("ƽ��ͶӰ");
		else
			//currentModeLabel->setText(QString(objectBased ? "object" : "viewer") + QString("-based perspective"));
			currentModeLabel->setText(QString(objectBased ? "������" : "�Թ۲���") + QString("-Ϊ���ĵ�͸��ͶӰ"));

		rotationCenterFrame->setEnabled(objectBased);
		pivotPickingToolButton->setEnabled(objectBased);
		eyePositionFrame->setEnabled(perspective);
	}
}
//=============================================updateViewMatrix======================================================//
void ccCameraParamEditDlg::updateViewMatrix(const ccGLMatrixd&)
{
	if (m_associatedWin)
		initWithMatrix(m_associatedWin->getBaseViewMat());
}

//================================================initWithMatrix========================================================//
void ccCameraParamEditDlg::initWithMatrix(const ccGLMatrixd& mat)
{
	//������ת���������ǶȲ�����һ��ƽ������
	double phi=0, theta=0, psi=0;
	CCVector3d trans;
	mat.getParameters(phi,theta,psi,trans);

	//to avoid retro-action
	ccGLWindow* win = m_associatedWin;
	m_associatedWin = 0; // �ɾ���ĵ���ֵ��������GLWindow

	//�ֲ�����б任
	//�ı�Ƕ�phi
	phiSpinBox->blockSignals(true);
	phiSpinBox->setValue(CC_RAD_TO_DEG*phi);//���ýǶ�Phi �ı� slider �� spinBox��ֵ
	dPhiValueChanged(phiSpinBox->value());
	phiSpinBox->blockSignals(false);
	
	//�ı�Ƕ�psi
	psiSpinBox->blockSignals(true);
	psiSpinBox->setValue(CC_RAD_TO_DEG*psi);
	dPsiValueChanged(psiSpinBox->value());
	psiSpinBox->blockSignals(false);

	//�ñ�Ƕ�theta
	thetaSpinBox->blockSignals(true);
	thetaSpinBox->setValue(CC_RAD_TO_DEG*theta);
	dThetaValueChanged(thetaSpinBox->value());
	thetaSpinBox->blockSignals(false);

	m_associatedWin = win;
}

//===================================================initWith===========================================//
void ccCameraParamEditDlg::initWith(ccGLWindow* win)
{
	setEnabled(win != 0);
	if (!win)
		return;

	//update matrix (angles) // ���ݵ�ǰ���ڵ� ����slider �͡�spinBox��ֵ
	initWithMatrix(win->getBaseViewMat());

	//��ȡ�ӽǲ���
	const ccViewportParameters& params = m_associatedWin->getViewportParameters();

	//update view mode
	updateViewMode();

	//update pivot point
	updatePivotPoint(params.pivotPoint);

	//update camera center
	updateCameraCenter(params.cameraCenter);

	//update FOV
	updateWinFov(win->getFov());

	//update zNearCoef
	zNearHorizontalSlider->blockSignals(true);
	zNearHorizontalSlider->setValue(ZNearCoefToSliderPos(params.zNearCoef,zNearHorizontalSlider->maximum()));
	zNearHorizontalSlider->blockSignals(false);
}

//=================================================updateCameratCenter======================================//
void ccCameraParamEditDlg::updateCameraCenter(const CCVector3d& P)
{
	exDoubleSpinBox->blockSignals(true);
	eyDoubleSpinBox->blockSignals(true);
	ezDoubleSpinBox->blockSignals(true);
	exDoubleSpinBox->setValue(P.x);
	eyDoubleSpinBox->setValue(P.y);
	ezDoubleSpinBox->setValue(P.z);
	exDoubleSpinBox->blockSignals(false);
	eyDoubleSpinBox->blockSignals(false);
	ezDoubleSpinBox->blockSignals(false);
}

//=======================================================updatePivotPoint====================================//
void ccCameraParamEditDlg::updatePivotPoint(const CCVector3d& P)
{
	if (!m_associatedWin)return;

	rcxDoubleSpinBox->blockSignals(true);
	rcyDoubleSpinBox->blockSignals(true);
	rczDoubleSpinBox->blockSignals(true);
	rcxDoubleSpinBox->setValue(P.x);
	rcyDoubleSpinBox->setValue(P.y);
	rczDoubleSpinBox->setValue(P.z);
	rcxDoubleSpinBox->blockSignals(false);
	rcyDoubleSpinBox->blockSignals(false);
	rczDoubleSpinBox->blockSignals(false);
}

//==========================================================updateWinFov=====================================//
void ccCameraParamEditDlg::updateWinFov(float fov_deg)
{
	if (!m_associatedWin)
		return;

	fovDoubleSpinBox->blockSignals(true);
	fovDoubleSpinBox->setValue(fov_deg);
	fovDoubleSpinBox->blockSignals(false);
}

//�������ǶȻ�ȡ��ת����
ccGLMatrixd ccCameraParamEditDlg::getMatrix()
{
	double phi = 0, theta = 0, psi = 0;
	//��slider�л�ȡ�Ƕ�ֵ�����������
	phi		= CC_DEG_TO_RAD * phiSpinBox->value();
	psi		= CC_DEG_TO_RAD * psiSpinBox->value();
	theta	= CC_DEG_TO_RAD * thetaSpinBox->value();

	ccGLMatrixd mat;
	CCVector3d T(0,0,0);
	mat.initFromParameters(phi,theta,psi,T);

	return mat;
}
