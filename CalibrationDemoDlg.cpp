
// CalibrationDemoDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "CalibrationDemo.h"
#include "CalibrationDemoDlg.h"
#include "afxdialogex.h"

#include <opencv2\opencv.hpp>
using namespace cv;

#include <VimbaCPP\Include\VimbaCPP.h>
#include <VimbaCPP\Include\Camera.h>
#include <VimbaCPP\Include\Feature.h>
#include <VimbaCPP\Include\IFeatureObserver.h>
using namespace AVT::VmbAPI;

#include "Apicontroller.h"
using AVT::VmbAPI::Examples::ApiController;

#include "CameraObserver.h"
#include "FrameObserver.h"
#include "ErrorCodeToMessage.h"
#include "StreamSystemInfo.h"

#include <iostream>
#include <vector>
#include <atlimage.h>
#include <time.h>

#define IMG_WIDTH 1024
#define IMG_HEIGHT 544

UINT __cdecl ProjectThread(LPVOID pParam);
UINT __cdecl CameraThread(LPVOID pParam);
UINT __cdecl VideoThread(LPVOID pParam);

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

	// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CCalibrationDemoDlg 对话框



CCalibrationDemoDlg::CCalibrationDemoDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_CALIBRATIONDEMO_DIALOG, pParent)
	, m_x(0)
	, m_y(0)
	, m_width()
	, m_height(0)
	, m_rew(0)
	, m_reh(0)
	, m_orw(0)
	, m_orh(0)
	, slider_gain(0)
	, slider_exposure(0)
	, slider_threshold(0)
	, slider_intensity(0)
	, edit_gain(0)
	, edit_exposure(0)
	, edit_threshold(0)
	, edit_intensity(0)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_run = true;
	m_project = false;
	m_pause = false;
	m_shot = false;
	m_video = false;

	video = Mat(Size(2048, 1088), CV_8UC3);
}

void CCalibrationDemoDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT1, m_x);
	DDX_Text(pDX, IDC_EDIT2, m_y);
	DDX_Text(pDX, IDC_EDIT3, m_width);
	DDX_Text(pDX, IDC_EDIT4, m_height);

	DDX_Text(pDX, IDC_EDIT7, m_rew);
	//DDV_MinMaxInt(pDX, m_rew, 0, 2048);
	DDX_Text(pDX, IDC_EDIT8, m_reh);
	//DDV_MinMaxInt(pDX, m_reh, 0, 1088);

	DDX_Text(pDX, IDC_EDIT6, m_orw);
	DDX_Text(pDX, IDC_EDIT5, m_orh);

	DDX_Slider(pDX, IDC_SLIDER1, slider_gain);
	//DDV_MinMaxInt(pDX, slider_gain, 0, 26);
	DDX_Slider(pDX, IDC_SLIDER2, slider_exposure);
	//DDV_MinMaxInt(pDX, slider_exposure, 0, 100);
	DDX_Slider(pDX, IDC_SLIDER3, slider_threshold);
	//DDV_MinMaxInt(pDX, slider_threshold, 0, 255);
	DDX_Slider(pDX, IDC_SLIDER4, slider_intensity);
	//DDV_MinMaxInt(pDX, slider_intensity, 0, 10);

	DDX_Text(pDX, IDC_EDIT_GAIN, edit_gain);
	DDX_Text(pDX, IDC_EDIT_EXPOSURE, edit_exposure);
	DDX_Text(pDX, IDC_EDIT_THRESHOLD, edit_threshold);
	DDX_Text(pDX, IDC_EDIT_INTENSITY, edit_intensity);
}

BEGIN_MESSAGE_MAP(CCalibrationDemoDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_RESET, &CCalibrationDemoDlg::OnBnClickedButtonReset)
	ON_BN_CLICKED(IDC_BUTTON_X_SUBTRACT, &CCalibrationDemoDlg::OnBnClickedButtonXSubtract)
	ON_BN_CLICKED(IDC_BUTTON_X_PLUS, &CCalibrationDemoDlg::OnBnClickedButtonXPlus)
	ON_BN_CLICKED(IDC_BUTTON_Y_SUBTRACT, &CCalibrationDemoDlg::OnBnClickedButtonYSubtract)
	ON_BN_CLICKED(IDC_BUTTON_Y_PLUS, &CCalibrationDemoDlg::OnBnClickedButtonYPlus)
	ON_BN_CLICKED(IDC_BUTTON_PROJECT, &CCalibrationDemoDlg::OnBnClickedButtonProject)
	ON_BN_CLICKED(IDC_BUTTON_SHOT, &CCalibrationDemoDlg::OnBnClickedButtonShot)
	ON_BN_CLICKED(IDC_BUTTON_VIDEO, &CCalibrationDemoDlg::OnBnClickedButtonVideo)
	ON_BN_CLICKED(IDC_BUTTON_SAVE, &CCalibrationDemoDlg::OnBnClickedButtonSave)
	ON_EN_CHANGE(IDC_EDIT_GAIN, &CCalibrationDemoDlg::OnChangeEditGain)
	ON_EN_CHANGE(IDC_EDIT_EXPOSURE, &CCalibrationDemoDlg::OnChangeEditExposure)
	ON_EN_CHANGE(IDC_EDIT_THRESHOLD, &CCalibrationDemoDlg::OnChangeEditThreshold)
	ON_EN_CHANGE(IDC_EDIT_INTENSITY, &CCalibrationDemoDlg::OnChangeEditIntensity)
	ON_WM_HSCROLL()
	ON_BN_CLICKED(IDC_BUTTON_PAUSE, &CCalibrationDemoDlg::OnBnClickedButtonPause)
	ON_BN_CLICKED(IDC_BUTTON_EXIT, &CCalibrationDemoDlg::OnBnClickedButtonExit)
END_MESSAGE_MAP()


// CCalibrationDemoDlg 消息处理程序

BOOL CCalibrationDemoDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码
	GetDlgItem(IDC_STATIC_SHOW)->SetWindowPos(NULL, 10, 10, IMG_WIDTH, IMG_HEIGHT, NULL);

	GetDlgItem(IDC_BUTTON_PAUSE)->EnableWindow(FALSE);
	GetDlgItem(IDC_BUTTON_PROJECT)->EnableWindow(FALSE);

	Mat img(Size(IMG_WIDTH, IMG_HEIGHT), CV_8UC3);
	namedWindow("show");
	HWND hWnd = (HWND)cvGetWindowHandle("show");
	HWND hParent = ::GetParent(hWnd);
	::SetParent(hWnd, GetDlgItem(IDC_STATIC_SHOW)->m_hWnd);
	::ShowWindow(hParent, SW_HIDE);
	img.setTo(Scalar(0x6e, 0x6e, 0x6e));
	imshow("show", img);

	ReadParam();
	AttachDisplay();

	namedWindow("project", CV_WINDOW_NORMAL);
	//namedWindow("project");
	img.setTo(Scalar(0x6e, 0x6e, 0x6e));
	imshow("project", img);
	waitKey(100);
	HWND hShowWnd = (HWND)cvGetWindowHandle("project");
	HWND hParentProject = ::GetParent(hShowWnd);
	setWindowProperty("project", CV_WND_PROP_FULLSCREEN, CV_WINDOW_FULLSCREEN);//1
	::SetWindowPos(hParentProject, HWND_TOPMOST, GetSystemMetrics(SM_CXSCREEN), 0, -1, -1, SWP_NOSIZE);

	GetDlgItem(IDC_BUTTON_PROJECT)->EnableWindow(TRUE);
	//Sleep(100);
	//pCameraThread = AfxBeginThread((AFX_THREADPROC)CameraThread, this, 0U, 0UL, 0, 0);
	Sleep(100);
	pProjectThread = AfxBeginThread((AFX_THREADPROC)ProjectThread, this, 0U, 0UL, 0, 0);
	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CCalibrationDemoDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CCalibrationDemoDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CCalibrationDemoDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CCalibrationDemoDlg::OnBnClickedButtonReset()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData(TRUE);

	if ((m_x + m_rew) > 2048)
	{
		m_rew = 2048 - m_x;
	}

	if ((m_y + m_reh) > 1088)
	{
		m_reh = 1088 - m_y;
	}

	m_width = m_rew;
	m_height = m_reh;
	UpdateData(FALSE);
}


void CCalibrationDemoDlg::OnBnClickedButtonXSubtract()
{
	// TODO: 在此添加控件通知处理程序代码
	if (m_x > 0)
	{
		m_x = m_x - 1;
	}
	UpdateData(FALSE);
}


void CCalibrationDemoDlg::OnBnClickedButtonXPlus()
{
	// TODO: 在此添加控件通知处理程序代码
	if (m_x < 2047)
	{
		m_x = m_x + 1;
	}
	UpdateData(FALSE);
}


void CCalibrationDemoDlg::OnBnClickedButtonYSubtract()
{
	// TODO: 在此添加控件通知处理程序代码
	if (m_y > 0)
	{
		m_y = m_y - 1;
	}
	UpdateData(FALSE);
}


void CCalibrationDemoDlg::OnBnClickedButtonYPlus()
{
	// TODO: 在此添加控件通知处理程序代码
	if (m_y < 1087)
	{
		m_y = m_y + 1;
	}
	UpdateData(FALSE);
}


void CCalibrationDemoDlg::OnBnClickedButtonProject()
{
	// TODO: 在此添加控件通知处理程序代码
	if (m_project == false)
	{
		m_project = true;
		GetDlgItem(IDC_BUTTON_PROJECT)->SetWindowTextW(L"Stop");
		GetDlgItem(IDC_BUTTON_PAUSE)->EnableWindow(TRUE);
	}
	else
	{
		m_project = false;
		m_pause = false;
		GetDlgItem(IDC_BUTTON_PROJECT)->SetWindowTextW(L"Project");
		GetDlgItem(IDC_BUTTON_PAUSE)->EnableWindow(FALSE);
	}
}


void CCalibrationDemoDlg::OnBnClickedButtonPause()
{
	// TODO: 在此添加控件通知处理程序代码
	if (m_pause == false)
	{
		m_pause = true;
		GetDlgItem(IDC_BUTTON_PAUSE)->SetWindowTextW(L"Continue");
		//	GetDlgItem(IDC_BUTTON_PROJECT)->EnableWindow(FALSE);
	}
	else
	{
		m_pause = false;
		GetDlgItem(IDC_BUTTON_PAUSE)->SetWindowTextW(L"Pause");
		//	GetDlgItem(IDC_BUTTON_PROJECT)->EnableWindow(FALSE);
	}
}



void CCalibrationDemoDlg::OnBnClickedButtonShot()
{
	// TODO: 在此添加控件通知处理程序代码
	m_shot = true;
}


void CCalibrationDemoDlg::OnBnClickedButtonVideo()
{
	// TODO: 在此添加控件通知处理程序代码
	if (m_video == false)
	{
		m_video = true;
		pVideoThread = AfxBeginThread((AFX_THREADPROC)VideoThread, this, 0U, 0UL, 0, 0);
		GetDlgItem(IDC_BUTTON_VIDEO)->SetWindowTextW(L"End");
	}
	else
	{
		m_video = false;
		WaitForSingleObject(pVideoThread->m_hThread, INFINITE);
		GetDlgItem(IDC_BUTTON_VIDEO)->SetWindowTextW(L"Video");
	}
}


void CCalibrationDemoDlg::OnBnClickedButtonSave()
{
	// TODO: 在此添加控件通知处理程序代码
	WriteParam();
}


void CCalibrationDemoDlg::OnBnClickedButtonExit()
{
	// TODO: 在此添加控件通知处理程序代码
	if (m_video == true)
	{
		m_video = false;
		WaitForSingleObject(pVideoThread->m_hThread, INFINITE);
	}
	Sleep(100);
	m_run = false;
	WaitForSingleObject(pProjectThread->m_hThread, INFINITE);
	CDialog::OnCancel();
}


void CCalibrationDemoDlg::OnChangeEditGain()
{
	// TODO:  如果该控件是 RICHEDIT 控件，它将不
	// 发送此通知，除非重写 CDialogEx::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。

	// TODO:  在此添加控件通知处理程序代码
	UpdateData(TRUE);

	if (edit_gain > 26)
	{
		edit_gain = 26;
	}
	else if (edit_gain < 0)
	{
		edit_gain = 0;
	}

	slider_gain = edit_gain * 3;
	UpdateData(FALSE);
}


void CCalibrationDemoDlg::OnChangeEditExposure()
{
	// TODO:  如果该控件是 RICHEDIT 控件，它将不
	// 发送此通知，除非重写 CDialogEx::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。

	// TODO:  在此添加控件通知处理程序代码
	UpdateData(TRUE);

	if (edit_exposure > 100)
	{
		edit_exposure = 100;
	}
	else if (edit_exposure < 0)
	{
		edit_exposure = 0;
	}

	slider_exposure = edit_exposure;
	UpdateData(FALSE);
}


void CCalibrationDemoDlg::OnChangeEditThreshold()
{
	// TODO:  如果该控件是 RICHEDIT 控件，它将不
	// 发送此通知，除非重写 CDialogEx::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。

	// TODO:  在此添加控件通知处理程序代码
	UpdateData(TRUE);

	if (edit_threshold > 250)
	{
		edit_threshold = 250;
	}
	else if (edit_threshold < 0)
	{
		edit_threshold = 0;
	}

	slider_threshold = edit_threshold / 2.5;
	UpdateData(FALSE);
}


void CCalibrationDemoDlg::OnChangeEditIntensity()
{
	// TODO:  如果该控件是 RICHEDIT 控件，它将不
	// 发送此通知，除非重写 CDialogEx::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。

	// TODO:  在此添加控件通知处理程序代码
	UpdateData(TRUE);

	if (edit_intensity > 10)
	{
		edit_intensity = 10;
	}
	else if (edit_intensity < 0)
	{
		edit_intensity = 0;
	}

	slider_intensity = edit_intensity * 10;
	UpdateData(FALSE);
}

void CCalibrationDemoDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	CSliderCtrl *pSlidCtrl1 = (CSliderCtrl*)GetDlgItem(IDC_SLIDER1);
	CSliderCtrl *pSlidCtrl2 = (CSliderCtrl*)GetDlgItem(IDC_SLIDER2);
	CSliderCtrl *pSlidCtrl3 = (CSliderCtrl*)GetDlgItem(IDC_SLIDER3);
	CSliderCtrl *pSlidCtrl4 = (CSliderCtrl*)GetDlgItem(IDC_SLIDER4);

	slider_gain = pSlidCtrl1->GetPos();
	slider_exposure = pSlidCtrl2->GetPos();
	slider_threshold = pSlidCtrl3->GetPos();
	slider_intensity = pSlidCtrl4->GetPos();

	if (slider_gain > 78)
	{
		slider_gain = 78;
	}
	else if (slider_gain < 0)
	{
		slider_gain = 0;
	}

	if (slider_exposure > 100)
	{
		slider_exposure = 100;
	}
	else if (slider_exposure < 0)
	{
		slider_exposure = 0;
	}

	if (slider_threshold > 100)
	{
		slider_threshold = 100;
	}
	else if (slider_threshold < 0)
	{
		slider_threshold = 0;
	}

	if (slider_intensity > 100)
	{
		slider_intensity = 100;
	}
	else if (slider_intensity < 0)
	{
		slider_intensity = 0;
	}

	edit_gain = slider_gain / 3;
	edit_exposure = slider_exposure;
	edit_threshold = slider_threshold*2.5;
	edit_intensity = slider_intensity / 10;
	UpdateData(FALSE);

	CDialogEx::OnHScroll(nSBCode, nPos, pScrollBar);
}



void CCalibrationDemoDlg::ReadParam()
{
	m_x = GetPrivateProfileInt(L"PARAMETERS", L"X", 0, L".\\Config.ini");
	m_y = GetPrivateProfileInt(L"PARAMETERS", L"Y", 0, L".\\Config.ini");
	m_width = GetPrivateProfileInt(L"PARAMETERS", L"Width", 0, L".\\Config.ini");
	m_height = GetPrivateProfileInt(L"PARAMETERS", L"Height", 0, L".\\Config.ini");

	edit_gain = GetPrivateProfileInt(L"CAMERA", L"Gain", 0, L".\\Config.ini");
	edit_exposure = GetPrivateProfileInt(L"CAMERA", L"Exposure", 0, L".\\Config.ini");
	edit_threshold = GetPrivateProfileInt(L"CAMERA", L"Threshold", 0, L".\\Config.ini");
	edit_intensity = GetPrivateProfileInt(L"CAMERA", L"Intensity", 0, L".\\Config.ini");

	m_orw = GetPrivateProfileInt(L"IMAGE", L"IMG_WIDTH", 0, L".\\Config.ini");
	m_orh = GetPrivateProfileInt(L"IMAGE", L"IMG_HEIGHT", 0, L".\\Config.ini");

	m_rew = m_width;
	m_reh = m_height;

	slider_gain = edit_gain * 3;
	slider_exposure = edit_exposure;
	slider_threshold = edit_threshold / 2.5;
	slider_intensity = edit_intensity * 10;
	UpdateData(FALSE);
}


void CCalibrationDemoDlg::WriteParam()
{
	CString str = L"";

	str.Format(L"%d", m_x);
	WritePrivateProfileString(L"PARAMETERS", L"X", str, L".\\Config.ini");
	str.Format(L"%d", m_y);
	WritePrivateProfileString(L"PARAMETERS", L"Y", str, L".\\Config.ini");
	str.Format(L"%d", m_width);
	WritePrivateProfileString(L"PARAMETERS", L"Width", str, L".\\Config.ini");
	str.Format(L"%d", m_height);
	WritePrivateProfileString(L"PARAMETERS", L"Height", str, L".\\Config.ini");


	str.Format(L"%d", edit_gain);
	WritePrivateProfileString(L"CAMERA", L"Gain", str, L".\\Config.ini");
	str.Format(L"%d", edit_exposure);
	WritePrivateProfileString(L"CAMERA", L"Exposure", str, L".\\Config.ini");
	str.Format(L"%d", edit_threshold);
	WritePrivateProfileString(L"CAMERA", L"Threshold", str, L".\\Config.ini");
	str.Format(L"%d", edit_intensity);
	WritePrivateProfileString(L"CAMERA", L"Intensity", str, L".\\Config.ini");
}


void CCalibrationDemoDlg::AttachDisplay()
{
	BOOL            FoundSecondaryDisp = FALSE;
	DWORD           DispNum = 0;
	DISPLAY_DEVICE  DisplayDevice;
	LONG            Result;
	//	TCHAR           szTemp[200];
	int             i = 0;
	DEVMODE   defaultMode;

	// initialize DisplayDevice
	ZeroMemory(&DisplayDevice, sizeof(DisplayDevice));
	DisplayDevice.cb = sizeof(DisplayDevice);

	// get all display devices
	while (EnumDisplayDevices(NULL, DispNum, &DisplayDevice, 0))
	{
		ZeroMemory(&defaultMode, sizeof(DEVMODE));
		defaultMode.dmSize = sizeof(DEVMODE);
		if (!EnumDisplaySettings((LPCWSTR)DisplayDevice.DeviceName, ENUM_REGISTRY_SETTINGS, &defaultMode))
			OutputDebugString(L"Store default failed\n");

		if (!(DisplayDevice.StateFlags & DISPLAY_DEVICE_ATTACHED_TO_DESKTOP))
		{
			DEVMODE    DevMode;
			DevMode = defaultMode;
			DevMode.dmFields |= DM_POSITION | DM_PELSHEIGHT | DM_PELSWIDTH;
			DevMode.dmPelsWidth = GetSystemMetrics(SM_CXSCREEN);
			DevMode.dmPelsHeight = GetSystemMetrics(SM_CYSCREEN);
			DevMode.dmPosition.x = GetSystemMetrics(SM_CXSCREEN);
			DevMode.dmPosition.y = 0;
			DevMode.dmSize = sizeof(DevMode);

			Result = ChangeDisplaySettingsEx((LPCWSTR)DisplayDevice.DeviceName,
				&DevMode,
				NULL,
				CDS_UPDATEREGISTRY | CDS_RESET | CDS_GLOBAL,
				NULL);
			ChangeDisplaySettingsEx(NULL, NULL, NULL, NULL, NULL);
		}

		// Reinit DisplayDevice just to be extra clean
		ZeroMemory(&DisplayDevice, sizeof(DisplayDevice));
		DisplayDevice.cb = sizeof(DisplayDevice);
		DispNum++;
	}
}


UINT __cdecl ProjectThread(LPVOID pParam)
{
	CCalibrationDemoDlg *p_this = (CCalibrationDemoDlg *)pParam;

	ApiController m_ApiController;
	FeaturePtr feature;
	CameraPtrVector cameras;
	CameraPtr camera;

	std::string CameraID = "";
	Mat frame;
	Mat fluo;
	Mat show;
	Mat temp;

	Mat project;
	Mat last;
	Mat green;
	std::vector<Mat> bgr;

	if (VmbErrorSuccess == m_ApiController.StartUp())
	{
		cameras = m_ApiController.GetCameraList();
		while (cameras.empty())
		{
			int order = AfxMessageBox(L"Cannot find camera, please retry!", MB_RETRYCANCEL);
			if (order == IDRETRY)
			{
				cameras = m_ApiController.GetCameraList();
			}
			else
			{
				m_ApiController.ShutDown();
				AfxMessageBox(L"Please exit and check your camera!");
				return 0;
			}
		}
		cameras[0]->GetID(CameraID);
		camera = cameras[0];

		if (VmbErrorSuccess == m_ApiController.StartContinuousImageAcquisition(CameraID))
		{
			waitKey(30);
			/////////////////////////////////////////////////////////////////////////////////////////////////////////////
			//VideoCapture cap(0);
			//Mat color;
			////////////////////////
			while (p_this->m_run == true)
			{
				//setting
				camera->GetFeatureByName("Gain", feature);
				feature->SetValue((float)p_this->edit_gain);
				camera->GetFeatureByName("ExposureTimeAbs", feature);
				feature->SetValue((float)(p_this->edit_exposure * 1000));
				waitKey(5);
				//////////////////////////////////////
				FramePtr pFrame = m_ApiController.GetFrame();
				if (!SP_ISNULL(pFrame))
				{
					VmbUchar_t *pBuffer;
					if (VmbErrorSuccess == pFrame->GetImage(pBuffer))
					{
						IplImage *head = cvCreateImageHeader(cvSize(2048, 1088), 8, 1);
						cvSetData(head, pBuffer, 2048);
						frame = cvarrToMat(head);
						//cap >> color;
						//cvtColor(color, frame, CV_RGB2GRAY);
						temp = frame;
					}
					else
					{
						AfxMessageBox(L"Cann't get buffer!");
					}
				}
				else
				{
					//AfxMessageBox(L"Data is not completed!");
					frame = temp;
				}
				/////////////////////////////////////////////////////////////
				//threshold
				threshold(frame, frame, (double)p_this->edit_threshold, (double)255, THRESH_TOZERO);

				fluo = frame;
				//p_this->video = frame;
				//screen image
				//rectangle(fluo, Point(p_this->m_x, p_this->m_y), Point((p_this->m_x + p_this->m_width), (p_this->m_y + p_this->m_height)), Scalar(255, 255, 255), 1, 8, 0);
				resize(fluo, show, Size(IMG_WIDTH, IMG_HEIGHT));
				imshow("show", show);

				//m_ApiController.QueueFrame(pFrame);
				//////////////////////////////////////////////////////////////
				if (p_this->m_shot == true)
				{
					time_t num;
					time(&num);
					char strtemp[20];
					sprintf_s(strtemp, "%d", num);

					std::string name = "";
					name = name + "Pictures//" + strtemp + ".png";

					imwrite(name, frame);
					p_this->m_shot = false;
				}
				////////////////////////////////////////////////////////
				if (p_this->m_project == true)
				{
					if (p_this->m_pause == true)
					{
						green = last;
					}
					else
					{
						project = Mat(frame, Rect(p_this->m_x, p_this->m_y, p_this->m_width, p_this->m_height));

						Mat zeros(project.size(), CV_8UC1);
						zeros.setTo(0);

						//threshold
						//threshold(project, project, (double)p_this->edit_threshold, (double)255, THRESH_TOZERO);

						//strengthen
						double min, max;
						minMaxIdx(project, &min, &max);
						project = project + p_this->edit_intensity*(max - min) / 10;

						//balck->green
						bgr.clear();
						bgr.push_back(zeros);
						bgr.push_back(project);
						bgr.push_back(zeros);
						merge(bgr, green);

						//record the last image
						last = green;
					}

					//recyify the green image to satify the pc_screen and keep the image unchanged when it is projected in projector
					imshow("project", green);
				}
				else
				{
					Mat img(Size(IMG_WIDTH, IMG_HEIGHT), CV_8UC3);
					img.setTo(Scalar(0x6e, 0x6e, 0x6e));
					imshow("project", img);
				}
				////////////////////////////////////////////////////////////////////
				m_ApiController.QueueFrame(pFrame);
			}
			////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			m_ApiController.ShutDown();
			m_ApiController.StopContinuousImageAcquisition();
			m_ApiController.ClearFrameQueue();
		}
		else
		{
			AfxMessageBox(L"Cann't get imge data!");
			m_ApiController.ShutDown();
			return 0;
		}
	}
	else
	{
		AfxMessageBox(L"Cann't open API!");
		return 0;
	}

	return 0;
}


UINT __cdecl VideoThread(LPVOID pParam)
{
	CCalibrationDemoDlg *p_this = (CCalibrationDemoDlg *)pParam;
	Mat video;

	time_t num;
	time(&num);
	char strtemp[20];
	sprintf_s(strtemp, "%d", num);

	std::string name = "";
	name = name + "Videos//" + strtemp + ".avi";

	video = p_this->video.clone();
	Size size = video.size();
	VideoWriter writer = VideoWriter(name, CV_FOURCC('X', 'V', 'I', 'D'), 24, size);

	while (p_this->m_video == true)
	{
		video = p_this->video.clone();
		writer << video;
		waitKey(10);
	}

	writer.release();
	return 0;
}