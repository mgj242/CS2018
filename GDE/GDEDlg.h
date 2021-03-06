#pragma once


#include "ColorListBox.h"

#include "Controller.h"
#include "SerialPort.h"


class CGDEDlg : public CDialogEx
{
public:
    // Construction

	CGDEDlg(CWnd* pParent = nullptr);


    // Dialog Data

#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_GDE_DIALOG };
#endif


protected:
    // Overrides

	virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV support


protected:
	// Message handling

    // Dialog
    afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
    afx_msg void OnGetMinMaxInfo(MINMAXINFO* lpMMI);
    virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
    afx_msg void OnTimer(UINT_PTR nIDEvent);

    // Configuration
    afx_msg void OnBnClickedButtonConnect();
    afx_msg void OnBnClickedCheckInitialIndoorLights();
    afx_msg void OnBnClickedCheckInitialOutdoorLights();
    afx_msg void OnCbnSelchangeComboInitialDoorPosition();
    afx_msg void OnEnChangeEditNoOfMageticStripes();
    afx_msg void OnEnChangeEditSecondsToOpen();
    afx_msg void OnEnChangeEditSecondsToClose();

    // Control
    afx_msg void OnBnClickedButtonFullClose();
    afx_msg void OnBnClickedButtonFullOpen();
    afx_msg void OnBnClickedCheckIndoorLights();
    afx_msg void OnBnClickedCheckOutdoorLights();
    afx_msg void OnBnClickedCheckSimulateBlockage();
    afx_msg void OnBnClickedButtonStepUp();
    afx_msg void OnBnClickedButtonStepDown();
    afx_msg void OnBnClickedButtonStop();

    // Log
    afx_msg void OnBnClickedButtonLogClear();
    afx_msg void OnBnClickedCheckFollow();
    afx_msg void OnCbnSelchangeComboLogFilter();

	DECLARE_MESSAGE_MAP()

private:
    // Constants


    enum { c_tickTimerInterval = 50/*ms*/ };


    // Implementation

    int _parseEditBoxNumber(const CEdit&) const;
    LPCTSTR _toTmpString(int value) const;

    void _enableConfigurationControls(bool enable);
    void _populateInitialDoorPositionComboBox();
    void _updateControlAndStatus();


    // Attributes

    HICON m_hIcon;

    // Configuration
    CComboBox m_comPortComboBox;
    CEdit m_noOfMagneticStripesEdit;
    CEdit m_secondsToOpenEdit;
    CEdit m_secondsToCloseEdit;
    CComboBox m_initialDoorPositionComboBox;

    // Control
    CButton m_connectButton;
    CButton m_stopButton;
    CButton m_fullOpenButton;
    CButton m_fullCloseButton;
    CButton m_stepUpButton;
    CButton m_stepDownButton;
    CButton m_simulateBlockageButton;
    CButton m_indoorLightsButton;
    CButton m_outdoorLightsButton;

    // Log
    CComboBox m_logFilterComboBox;
    CButton m_logFollowCheckBox;
    CColorListBox m_logListBox;

    // Status
    CStatic m_commChanelStatusLabel;
    CStatic m_motorStatusLabel;
    CStatic m_doorStatusLabel;
    CStatic m_indoorLightsStatusLabel;
    CStatic m_outdoorLightsStatusLabel;

    // Model
    Application::Controller m_controller;
    Application::SerialPort m_serialPort;

    UINT_PTR m_tickTimer;
};
