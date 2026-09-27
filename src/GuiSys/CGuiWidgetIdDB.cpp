#include <GuiSys/CGuiWidget.hpp>
#include <GuiSys/CGuiWidgetIdDB.hpp>

CGuiWidgetIdDB::CGuiWidgetIdDB() {
  mDb.reserve(4);
  AddWidget(rstl::string_l("kGSYS_DummyWidgetID"));
  AddWidget(rstl::string_l("kGSYS_HeadWidgetID"));
  AddWidget(rstl::string_l("kGSYS_DefaultCameraID"));
  AddWidget(rstl::string_l("kGSYS_DefaultLightID"));
}

void CGuiWidgetIdDB::Reserve(int size) { mDb.reserve(size + mDb.size()); }

const short CGuiWidgetIdDB::AddWidget(const rstl::string& name) {
  short id = FindWidgetID(name);
  if (id == CGuiWidget::InvalidWidgetId()) {
    mDb.push_back(name);
    id = mDb.size() - 1;
  }

  return id;
}

const short CGuiWidgetIdDB::FindWidgetID(const rstl::string& name) const {
  for (int i = 0; i < mDb.size(); ++i) {
    if (mDb[i] == name) {
      return i;
    }
  }

  return CGuiWidget::InvalidWidgetId();
}

static void keep_string() { static const char* derp = "not found"; }
