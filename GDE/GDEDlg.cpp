#include "stdafx.h"

#include "afxdialogex.h"

#include "GDE.h"
#include "GDEDlg.h"
#include "ReentrancyGuard.h"

#include "Log.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


using namespace Application;


// Construction & destruction


CGDEDlg::CGDEDlg(CWnd* pParent/*=nullptr*/) :
    CDialogEx(IDD_GDE_DIALOG, pParent)
,   m_tickTimer(0)
{
    m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}


// Protected Overrides


void CGDEDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);

    // Configuration
    DDX_Control(pDX, IDC_COMBO_INITIAL_DOOR_POSITION, m_initialDoorPositionComboBox);
    DDX_Control(pDX, IDC_EDIT_NO_OF_MAGETIC_STRIPES, m_noOfMagneticStripesEdit);
    DDX_Control(pDX, IDC_EDIT_SECONDS_TO_CLOSE, m_secondsToCloseEdit);
    DDX_Control(pDX, IDC_EDIT_SECONDS_TO_OPEN, m_secondsToOpenEdit);

    // Control
    DDX_Control(pDX, IDC_BUTTON_FULL_CLOSE, m_fullCloseButton);
    DDX_Control(pDX, IDC_BUTTON_FULL_OPEN, m_fullOpenButton);
    DDX_Control(pDX, IDC_BUTTON_STEP_UP, m_stepUpButton);
    DDX_Control(pDX, IDC_BUTTON_STEP_DOWN, m_stepDownButton);
    DDX_Control(pDX, IDC_BUTTON_STOP, m_stopButton);
    DDX_Control(pDX, IDC_CHECK_INDOOR_LIGHTS, m_indoorLightsButton);
    DDX_Control(pDX, IDC_CHECK_OUTDOOR_LIGHTS, m_outdoorLightsButton);
    DDX_Control(pDX, IDC_CHECK_SIMULATE_BLOCKAGE, m_simulateBlockageButton);

    // Log
    DDX_Control(pDX, IDC_CHECK_FOLLOW, m_logFollowCheckBox);
    DDX_Control(pDX, IDC_COMBO_COM_PORT, m_comPortComboBox);
    DDX_Control(pDX, IDC_COMBO_LOG_FILTER, m_logFilterComboBox);
    DDX_Control(pDX, IDC_LIST_LOG, m_logListBox);
    DDX_Control(pDX, IDC_BUTTON_CONNECT, m_connectButton);

    // Status
    DDX_Control(pDX, IDC_STATIC_COMMUNICATION_CHANNEL_STATUS, m_commChanelStatusLabel);
    DDX_Control(pDX, IDC_STATIC_MOTOR_TEXT, m_motorStatusLabel);
    DDX_Control(pDX, IDC_STATIC_DOOR_POSITION_TEXT, m_doorStatusLabel);
    DDX_Control(pDX, IDC_STATIC_INDOOR_LIGHTS_STATUS, m_indoorLightsStatusLabel);
    DDX_Control(pDX, IDC_STATIC_OUTDOOR_LIGHTS_STATUS, m_outdoorLightsStatusLabel);
}


// Message handling


BEGIN_MESSAGE_MAP(CGDEDlg, CDialogEx)
    // Dialog
    ON_WM_CTLCOLOR()
    ON_WM_GETMINMAXINFO()
    ON_WM_LBUTTONDOWN()
    ON_WM_PAINT()
    ON_WM_QUERYDRAGICON()
    ON_WM_TIMER()

    // Configuration
    ON_BN_CLICKED(IDC_BUTTON_CONNECT, &CGDEDlg::OnBnClickedButtonConnect)
    ON_CBN_SELCHANGE(IDC_COMBO_INITIAL_DOOR_POSITION, &CGDEDlg::OnCbnSelchangeComboInitialDoorPosition)
    ON_EN_CHANGE(IDC_EDIT_NO_OF_MAGETIC_STRIPES, &CGDEDlg::OnEnChangeEditNoOfMageticStripes)
    ON_EN_CHANGE(IDC_EDIT_SECONDS_TO_OPEN, &CGDEDlg::OnEnChangeEditSecondsToOpen)
    ON_EN_CHANGE(IDC_EDIT_SECONDS_TO_CLOSE, &CGDEDlg::OnEnChangeEditSecondsToClose)

    // Control
    ON_BN_CLICKED(IDC_BUTTON_FULL_CLOSE, &CGDEDlg::OnBnClickedButtonFullClose)
    ON_BN_CLICKED(IDC_BUTTON_FULL_OPEN, &CGDEDlg::OnBnClickedButtonFullOpen)
    ON_BN_CLICKED(IDC_BUTTON_STEP_UP, &CGDEDlg::OnBnClickedButtonStepUp)
    ON_BN_CLICKED(IDC_BUTTON_STEP_DOWN, &CGDEDlg::OnBnClickedButtonStepDown)
    ON_BN_CLICKED(IDC_BUTTON_STOP, &CGDEDlg::OnBnClickedButtonStop)
    ON_BN_CLICKED(IDC_CHECK_INDOOR_LIGHTS, &CGDEDlg::OnBnClickedCheckIndoorLights)
    ON_BN_CLICKED(IDC_CHECK_OUTDOOR_LIGHTS, &CGDEDlg::OnBnClickedCheckOutdoorLights)
    ON_BN_CLICKED(IDC_CHECK_SIMULATE_BLOCKAGE, &CGDEDlg::OnBnClickedCheckSimulateBlockage)

    // Log
    ON_BN_CLICKED(IDC_BUTTON_LOG_CLEAR, &CGDEDlg::OnBnClickedButtonLogClear)
    ON_BN_CLICKED(IDC_CHECK_FOLLOW, &CGDEDlg::OnBnClickedCheckFollow)
    ON_CBN_SELCHANGE(IDC_COMBO_LOG_FILTER, &CGDEDlg::OnCbnSelchangeComboLogFilter)
END_MESSAGE_MAP()


// Message handling - dialog


HBRUSH CGDEDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
    HBRUSH hbr;
    ATLASSERT(pWnd != nullptr);
    if (pWnd->GetDlgCtrlID() == IDC_LIST_LOG) {
        // change background color of log list box to black
        ATLASSERT(pDC != nullptr);
        pDC->SetDCBrushColor(RGB(0, 0, 0));
        pDC->SetBkMode(TRANSPARENT);
        hbr = static_cast<HBRUSH>(GetStockObject(DC_BRUSH));
    } else {
        hbr = CDialogEx::OnCtlColor(pDC, pWnd, nCtlColor);
    }
    return hbr;
}


void CGDEDlg::OnGetMinMaxInfo(MINMAXINFO* lpMMI)
{
    ATLASSERT(lpMMI);

    CDialog::OnGetMinMaxInfo(lpMMI);

    // limit resizing to vertical changes, no shrinking below original size
    static CRect s_rect;
    if (s_rect.IsRectNull()) {
        GetWindowRect(&s_rect);
    }
    if (!s_rect.IsRectEmpty()) {
        lpMMI->ptMaxSize.x = s_rect.Width();
        lpMMI->ptMaxTrackSize.x = s_rect.Width();
        lpMMI->ptMinTrackSize.x = s_rect.Width();
        lpMMI->ptMinTrackSize.y = s_rect.Height();
    }
}


BOOL CGDEDlg::OnInitDialog()
{
    CDialogEx::OnInitDialog();

    // Set the icon for this dialog
    SetIcon(m_hIcon, TRUE);     // Set big icon
    SetIcon(m_hIcon, FALSE);    // Set small icon

    // Initialize logging
    Log::getInstance().initialize(&m_logListBox, &m_logFilterComboBox, &m_logFollowCheckBox);

    // Populate and initialize COM port combobox
    while (m_comPortComboBox.GetCount() > 0)
        m_comPortComboBox.DeleteString(/*index*/0);
    const SerialPort::PortsAndNamesArray& portsAndNames = m_serialPort.enumerateComPorts();
    for (size_t i = 0; i < portsAndNames.size(); ++i) {
        m_comPortComboBox.AddString(portsAndNames[i].second.c_str());
        m_comPortComboBox.SetItemData(static_cast<int>(i), portsAndNames[i].first);
    }
    if (portsAndNames.size() > 0)
        m_comPortComboBox.SetCurSel(/*index*/0);

    // Set initial values of Configuration controls
    m_noOfMagneticStripesEdit.SetWindowTextW(_toTmpString(m_controller.getDoorStripesCount()));
    m_secondsToOpenEdit.SetWindowTextW(_toTmpString(m_controller.getSecondsToOpen()));
    m_secondsToCloseEdit.SetWindowTextW(_toTmpString(m_controller.getSecondsToClose()));
    _populateInitialDoorPositionComboBox();
    m_indoorLightsButton.SetCheck(m_controller.isIndoorLightsOn() ? BST_CHECKED : BST_UNCHECKED);
    m_outdoorLightsButton.SetCheck(m_controller.isOutdoorLightsOn() ? BST_CHECKED : BST_UNCHECKED);

    // update Control and Status controls, start tick timer
    _updateControlAndStatus();

    m_tickTimer = SetTimer(IDT_TICK, /*nElapse*/c_tickTimerInterval, /*timerHandler*/nullptr/*post WM_TIMER*/);
    if (m_tickTimer == 0)
        APP_LOG_ERROR(_T("Failed to create tick timer"));
    
    return TRUE;  // not setting the focus to a control
}


void CGDEDlg::OnPaint()
{
    if (IsIconic()) {
        CPaintDC dc(this); // device context for painting

        SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

        // Draw minimized application icon

        // Center icon in client rectangle
        const int cxIcon = GetSystemMetrics(SM_CXICON);
        const int cyIcon = GetSystemMetrics(SM_CYICON);
        CRect rect;
        GetClientRect(&rect);
        const int x = (rect.Width() - cxIcon + 1) / 2;
        const int y = (rect.Height() - cyIcon + 1) / 2;

        // Draw the icon
        dc.DrawIcon(x, y, m_hIcon);
    } else {
        CDialogEx::OnPaint();
    }
}

HCURSOR CGDEDlg::OnQueryDragIcon() // get the cursor to display while the user drags the minimized window
{
    return static_cast<HCURSOR>(m_hIcon);
}


void CGDEDlg::OnTimer(UINT_PTR nIDEvent)
{
    if (nIDEvent == IDT_TICK) {
        if (!m_controller.update())
            APP_LOG_ERROR(_T("Controller update failed"));
        _updateControlAndStatus();
    }
    CDialogEx::OnTimer(nIDEvent);
}


// Message handling - configuration


void CGDEDlg::OnBnClickedButtonConnect()
{
    if (m_controller.isConnected()) {
        m_controller.disconnect();
        m_connectButton.SetWindowTextW(_T("Connect"));
        m_serialPort.close();
        _enableConfigurationControls(true);
    } else {
        int comPortSelIx = m_comPortComboBox.GetCurSel();
        if (comPortSelIx >= 0) {
            UINT comPortIx = static_cast<UINT>(m_comPortComboBox.GetItemData(comPortSelIx));
            APP_LOG_DEBUG(_T("Using COM port #%u"), comPortIx);
            if (m_serialPort.open(comPortIx) && m_controller.connect(m_serialPort)) {
                m_connectButton.SetWindowTextW(_T("Disconnect"));
                _enableConfigurationControls(false);
            } else {
                APP_LOG_ERROR(_T("Unable to connect to COM port #%u"), comPortIx);
            }
        } else {
            APP_LOG_ERROR(_T("No COM port selected"));
        }
    }
}

void CGDEDlg::OnCbnSelchangeComboInitialDoorPosition()
{
    int value = m_initialDoorPositionComboBox.GetCurSel();
    if (value >= 0) {
        if (!m_controller.setDoorStripePosition(value)) {
            APP_LOG_ERROR(_T("Unable to change door position to stripe #%d"), value);
            value = m_controller.getDoorStripePosition();
            if (m_initialDoorPositionComboBox.SetCurSel(value) == CB_ERR) {
                APP_LOG_ERROR(_T("Unable to reset door position to original stripe #%d"), value);
            }
        }
    } else {
        APP_LOG_WARNING(_T("Unable to parse value of \"Initial door position\" edit box"));
    }
}

void CGDEDlg::OnEnChangeEditNoOfMageticStripes()
{
    REENTRANCY_GUARD();
    int value = _parseEditBoxNumber(m_noOfMagneticStripesEdit);
    if (value >= 0) {
        if (m_controller.setDoorStripesCount(value)) {
            _populateInitialDoorPositionComboBox();
        } else {
            APP_LOG_ERROR(_T("Unable to change door stripes count to %d"), value);
            value = m_controller.getDoorStripesCount();
            m_noOfMagneticStripesEdit.SetWindowTextW(_toTmpString(value));
        }
    } else {
        APP_LOG_ERROR(_T("Unable to parse value of \"No. of magnetic stripes\" edit box"));
    }
}


void CGDEDlg::OnEnChangeEditSecondsToClose()
{
    REENTRANCY_GUARD();
    int value = _parseEditBoxNumber(m_secondsToCloseEdit);
    if (value >= 0) {
        if (!m_controller.setSecondsToClose(value)) {
            APP_LOG_ERROR(_T("Unable to change door closing time to %d seconds"), value);
            value = m_controller.getSecondsToClose();
            m_secondsToCloseEdit.SetWindowTextW(_toTmpString(value));
        }
    } else {
        APP_LOG_ERROR(_T("Unable to parse value of \"Seconds to close\" edit box"));
    }
}

void CGDEDlg::OnEnChangeEditSecondsToOpen()
{
    REENTRANCY_GUARD();
    int value = _parseEditBoxNumber(m_secondsToOpenEdit);
    if (value >= 0) {
        if (!m_controller.setSecondsToOpen(value)) {
            APP_LOG_ERROR(_T("Unable to change door opening time to %d seconds"), value);
            value = m_controller.getSecondsToOpen();
            m_secondsToOpenEdit.SetWindowTextW(_toTmpString(value));
        }
    } else {
        APP_LOG_ERROR(_T("Unable to parse value of \"Seconds to open\" edit box"));
    }
}


// Message handling - control


void CGDEDlg::OnBnClickedButtonFullClose()
{
    m_controller.fullDown();
}

void CGDEDlg::OnBnClickedButtonFullOpen()
{
    m_controller.fullUp();
}

void CGDEDlg::OnBnClickedCheckIndoorLights()
{
    m_controller.switchIndoorLights(m_indoorLightsButton.GetCheck() == BST_CHECKED);
}

void CGDEDlg::OnBnClickedCheckOutdoorLights()
{
    m_controller.switchOutdoorLights(m_outdoorLightsButton.GetCheck() == BST_CHECKED);
}

void CGDEDlg::OnBnClickedCheckSimulateBlockage()
{
    m_controller.simulateBlockage(m_simulateBlockageButton.GetCheck() == BST_CHECKED);
}

void CGDEDlg::OnBnClickedButtonStepUp()
{
    m_controller.stepUp();
}

void CGDEDlg::OnBnClickedButtonStepDown()
{
    m_controller.stepDown();
}

void CGDEDlg::OnBnClickedButtonStop()
{
    m_controller.stop();
}


// Message handling - log


void CGDEDlg::OnBnClickedButtonLogClear()
{
    Log::getInstance().clear();
}

void CGDEDlg::OnBnClickedCheckFollow()
{
    Log::getInstance().setFollow(m_logFollowCheckBox.GetCheck() == BST_CHECKED);
}

void CGDEDlg::OnCbnSelchangeComboLogFilter()
{
    int selectedIx = m_logFilterComboBox.GetCurSel();
    if (selectedIx >= 0) {
        Log::getInstance().setFilter(static_cast<Log::Severity>(m_logFilterComboBox.GetItemData(selectedIx)));
    }
}


// Implementation


void CGDEDlg::_enableConfigurationControls(bool enable)
{
    m_comPortComboBox.EnableWindow(enable);
    m_noOfMagneticStripesEdit.EnableWindow(enable);
    m_secondsToOpenEdit.EnableWindow(enable);
    m_secondsToCloseEdit.EnableWindow(enable);
    m_initialDoorPositionComboBox.EnableWindow(enable);
}


int CGDEDlg::_parseEditBoxNumber(const CEdit& editBox) const
{
    int textLength = editBox.GetWindowTextLengthW();
    if (textLength > 0) {
        textLength += 1/*trailing NUL*/;
        try {
            LPTSTR buffer = new TCHAR[textLength];
            editBox.GetWindowTextW(buffer, textLength);
            int result = _tstoi(buffer);
            delete[] buffer;
            return result;
        }
        catch (const std::bad_alloc& error) {
            APP_LOG_ERROR(_T("Unable to allocate %lu characters for edit box text: %s"), textLength, error.what());
            return -1;
        }
    }
    return -1;
}


void CGDEDlg::_populateInitialDoorPositionComboBox()
{
    // delete contents
    while (m_initialDoorPositionComboBox.GetCount() > 0)
        m_initialDoorPositionComboBox.DeleteString(/*index*/0);
    // populate with 0 .. door stripes count
    for (int i = 0; i < m_controller.getDoorStripesCount(); ++i) {
        m_initialDoorPositionComboBox.AddString(_toTmpString(i));
    }
    if (m_controller.getDoorStripesCount() > 0)
        m_initialDoorPositionComboBox.SetCurSel(/*index*/0); // fully closed
}


LPCTSTR CGDEDlg::_toTmpString(int value) const
{
    static String result;
    OStringStream buf;
    buf << value;
    result = buf.str();
    return result.c_str();
}

void CGDEDlg::_updateControlAndStatus()
{
    bool isConnected = m_controller.isConnected();

    // Control
    m_stopButton.EnableWindow(isConnected && m_controller.isMotorRunning());
    m_fullOpenButton.EnableWindow(isConnected && !m_controller.isFullyOpened()
        && m_controller.getMotorState() != RunningUp && !m_controller.isBlocked());
    m_fullCloseButton.EnableWindow(isConnected && !m_controller.isFullyClosed()
        && m_controller.getMotorState() != RunningDown && !m_controller.isBlocked());
    m_stepUpButton.EnableWindow(isConnected && !m_controller.isFullyOpened()
        && m_controller.getMotorState() != SteppingUp && !m_controller.isBlocked());
    m_stepDownButton.EnableWindow(isConnected && !m_controller.isFullyClosed()
        && m_controller.getMotorState() != SteppingDown && !m_controller.isBlocked());

    m_simulateBlockageButton.SetCheck(m_controller.isBlocked() ? BST_CHECKED : BST_UNCHECKED);
    m_indoorLightsButton.SetCheck(m_controller.isIndoorLightsOn() ? BST_CHECKED : BST_UNCHECKED);
    m_outdoorLightsButton.SetCheck(m_controller.isOutdoorLightsOn() ? BST_CHECKED : BST_UNCHECKED);

    // Status
    m_commChanelStatusLabel.SetWindowTextW(isConnected ?
        m_controller.isCommunicationError() ? _T("Error") : _T("Connected") : _T("Disconnected"));

    const TCHAR* motorStatus = nullptr;
    switch (m_controller.getMotorState()) {
    case Stopped:       motorStatus = _T("Stopped"); break;
    case Blocked:       motorStatus = _T("Blocked"); break;
    case RunningDown:   motorStatus = _T("Running down"); break;
    case RunningUp:     motorStatus = _T("Running up"); break;
    case SteppingDown:  motorStatus = _T("Stepping down"); break;
    case SteppingUp:    motorStatus = _T("Stepping up"); break;
    default:
        ATLASSERT(false/*not implemented*/);
    }
    if (motorStatus != nullptr)
        m_motorStatusLabel.SetWindowTextW(motorStatus);

    OStringStream buf;
    buf.setf(std::ios::fixed, std::ios::floatfield);
    buf.precision(2);
    buf << m_controller.getDoorPercentPosition() << "% open, "
        << m_controller.getDoorStripePosition() << '/' << m_controller.getDoorStripesCount();
    m_doorStatusLabel.SetWindowTextW(buf.str().c_str());

    m_indoorLightsStatusLabel.SetWindowTextW(m_controller.isIndoorLightsOn() ? _T("On") : _T("Off"));
    m_outdoorLightsStatusLabel.SetWindowTextW(m_controller.isOutdoorLightsOn() ? _T("On") : _T("Off"));
}
