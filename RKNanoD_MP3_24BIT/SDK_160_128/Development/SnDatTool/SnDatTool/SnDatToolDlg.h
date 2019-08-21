
// SnDatToolDlg.h : ͷ�ļ�
//

#pragma once
//����sn�ļ��ṹ
#define SN_DAT_HEADER_TAG 0x46444E53
#define SN_MAX_LENGTH 60
#pragma pack(1)
typedef struct {
	UINT uiTag;//'SNDF'
	UINT uiHeaderSize;//size of header
	UINT uiTotal;//total of sn
	UINT uiFree;//count of available sn
	UINT uiSnLength;//must be smaller than 60 
	UINT uiSnDataCrc;//crc of sn
	BYTE snBitmap[1];
}STRUCT_SN_DAT_HEAD,*PSTRUCT_SN_DAT_HEAD;
#pragma pack()

// CSnDatToolDlg �Ի���
class CSnDatToolDlg : public CDialog
{
// ����
public:
	CSnDatToolDlg(CWnd* pParent = NULL);	// ��׼���캯��

// �Ի�������
	enum { IDD = IDD_SNDATTOOL_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��


// ʵ��
protected:
	HICON m_hIcon;

	// ���ɵ���Ϣӳ�亯��
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedBtnGenerate();
};
