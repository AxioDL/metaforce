#ifndef _CDVDREQUESTMANAGER
#define _CDVDREQUESTMANAGER

class CDvdRequestSys {
public:
  CDvdRequestSys() {
    if (mManagerInstalled != true) {
      mManagerInstalled = true;
    }
  }
  ~CDvdRequestSys() {
    if (mManagerInstalled == true) {
      mManagerInstalled = false;
    }
  }

private:
  static bool mManagerInstalled;
};

#endif // _CDVDREQUESTMANAGER
