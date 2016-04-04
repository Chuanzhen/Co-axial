
// CalibrationDemoDlg.h : ͷ�ļ�
//
#include <opencv2\opencv.hpp>
using namespace cv;

#pragma once


// CCalibrationDemoDlg �Ի���
class CCalibrationDemoDlg : public CDialogEx
{
// ����
public:
	CCalibrationDemoDlg(CWnd* pParent = NULL);	// ��׼���캯��

// �Ի�������
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_CALIBRATIONDEMO_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��


// ʵ��
protected:
	HICON m_hIcon;

	// ���ɵ���Ϣӳ�亯��
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()

public:
	int m_x;
	int m_y;
	int m_width;
	int m_height;

	int m_rew;
	int m_reh;

	int m_orw;
	int m_orh;

	int slider_gain;
	int slider_exposure;
	int slider_threshold;
	int slider_intensity;

	int edit_gain;
	int edit_exposure;
	int edit_threshold;
	int edit_intensity;

	bool m_run;
	bool m_project;
	bool m_pause;
	bool m_shot;
	bool m_video;

	afx_msg void OnBnClickedButtonReset();
	afx_msg void OnBnClickedButtonXSubtract();
	afx_msg void OnBnClickedButtonXPlus();
	afx_msg void OnBnClickedButtonYSubtract();
	afx_msg void OnBnClickedButtonYPlus();
	afx_msg void OnBnClickedButtonProject();
	afx_msg void OnBnClickedButtonPause();
	afx_msg void OnBnClickedButtonShot();
	afx_msg void OnBnClickedButtonVideo();
	afx_msg void OnBnClickedButtonSave();
	afx_msg void OnBnClickedButtonExit();

	void ReadParam();
	void WriteParam();
	void AttachDisplay();

	afx_msg void OnChangeEditGain();
	afx_msg void OnChangeEditExposure();
	afx_msg void OnChangeEditThreshold();
	afx_msg void OnChangeEditIntensity();
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);

	CWinThread* pProjectThread;
	CWinThread* pVideoThread;
	//CWinThread* pCameraThread;

	Mat video;
};
