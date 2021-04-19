/* Locales */
enum class ELocale {
  Invalid = -1,
  en_US,
  en_GB,
  ja_JP,
  MAXLocale
};
struct en_US { static constexpr auto Name = "en_US"sv; static constexpr auto FullName = "US English"sv; };
struct en_GB { static constexpr auto Name = "en_GB"sv; static constexpr auto FullName = "British English"sv; };
struct ja_JP { static constexpr auto Name = "ja_JP"sv; static constexpr auto FullName = "日本語"sv; };

using DefaultLocale = en_US;
template<typename L, typename K> struct Lookup {
  static_assert(!std::is_same_v<L, DefaultLocale>, "The default locale must translate all keys");
  static constexpr auto Value() { return Lookup<DefaultLocale, K>::Value(); }
};

/* Keys */
struct color {};
struct branch {};
struct commit {};
struct release {};
struct date {};
struct new_project {};
struct open_project {};
struct extract_game {};
struct name {};
struct type {};
struct size {};
struct directory {};
struct file {};
struct file_name {};
struct cancel {};
struct system_locations {};
struct recent_projects {};
struct recent_files {};
struct scroll_left {};
struct scroll_right {};
struct ok {};
struct boundary_action {};
struct split {};
struct join {};
struct hecl_project {};
struct no_access_as_dir {};
struct file_field_empty {};
struct overwrite_confirm {};
struct directory_field_empty {};
struct no_overwrite_file {};
struct no_overwrite_project {};
struct no_access_as_file {};
struct space_types {};
struct resource_browser {};
struct effect_editor {};
struct model_viewer {};
struct information_center {};
struct game_mode {};
struct version {};

/* en_US */
template<> struct Lookup<en_US, color> { static constexpr auto Value() { return FMT_STRING("Color"); } };
template<> struct Lookup<en_US, branch> { static constexpr auto Value() { return FMT_STRING("Branch"); } };
template<> struct Lookup<en_US, commit> { static constexpr auto Value() { return FMT_STRING("Commit"); } };
template<> struct Lookup<en_US, release> { static constexpr auto Value() { return FMT_STRING("Release"); } };
template<> struct Lookup<en_US, date> { static constexpr auto Value() { return FMT_STRING("Date"); } };
template<> struct Lookup<en_US, new_project> { static constexpr auto Value() { return FMT_STRING("New Project"); } };
template<> struct Lookup<en_US, open_project> { static constexpr auto Value() { return FMT_STRING("Open Project"); } };
template<> struct Lookup<en_US, extract_game> { static constexpr auto Value() { return FMT_STRING("Extract Game"); } };
template<> struct Lookup<en_US, name> { static constexpr auto Value() { return FMT_STRING("Name"); } };
template<> struct Lookup<en_US, type> { static constexpr auto Value() { return FMT_STRING("Type"); } };
template<> struct Lookup<en_US, size> { static constexpr auto Value() { return FMT_STRING("Size"); } };
template<> struct Lookup<en_US, directory> { static constexpr auto Value() { return FMT_STRING("Directory"); } };
template<> struct Lookup<en_US, file> { static constexpr auto Value() { return FMT_STRING("File"); } };
template<> struct Lookup<en_US, file_name> { static constexpr auto Value() { return FMT_STRING("File Name"); } };
template<> struct Lookup<en_US, cancel> { static constexpr auto Value() { return FMT_STRING("Cancel"); } };
template<> struct Lookup<en_US, system_locations> { static constexpr auto Value() { return FMT_STRING("System Locations"); } };
template<> struct Lookup<en_US, recent_projects> { static constexpr auto Value() { return FMT_STRING("Recent Projects"); } };
template<> struct Lookup<en_US, recent_files> { static constexpr auto Value() { return FMT_STRING("Recent Files"); } };
template<> struct Lookup<en_US, scroll_left> { static constexpr auto Value() { return FMT_STRING("Scroll Left"); } };
template<> struct Lookup<en_US, scroll_right> { static constexpr auto Value() { return FMT_STRING("Scroll Right"); } };
template<> struct Lookup<en_US, ok> { static constexpr auto Value() { return FMT_STRING("OK"); } };
template<> struct Lookup<en_US, boundary_action> { static constexpr auto Value() { return FMT_STRING("Boundary Action"); } };
template<> struct Lookup<en_US, split> { static constexpr auto Value() { return FMT_STRING("Split"); } };
template<> struct Lookup<en_US, join> { static constexpr auto Value() { return FMT_STRING("Join"); } };
template<> struct Lookup<en_US, hecl_project> { static constexpr auto Value() { return FMT_STRING("HECL Project"); } };
template<> struct Lookup<en_US, no_access_as_dir> { static constexpr auto Value() { return FMT_STRING("Unable to access '{}' as directory"); } };
template<> struct Lookup<en_US, file_field_empty> { static constexpr auto Value() { return FMT_STRING("Unable to save empty file"); } };
template<> struct Lookup<en_US, overwrite_confirm> { static constexpr auto Value() { return FMT_STRING("Overwrite '{}'?"); } };
template<> struct Lookup<en_US, directory_field_empty> { static constexpr auto Value() { return FMT_STRING("Unable to make empty-named directory"); } };
template<> struct Lookup<en_US, no_overwrite_file> { static constexpr auto Value() { return FMT_STRING("Unable to make directory over file"); } };
template<> struct Lookup<en_US, no_overwrite_project> { static constexpr auto Value() { return FMT_STRING("Unable to make project within existing project"); } };
template<> struct Lookup<en_US, no_access_as_file> { static constexpr auto Value() { return FMT_STRING("Unable to access '{}' as file"); } };
template<> struct Lookup<en_US, space_types> { static constexpr auto Value() { return FMT_STRING("Space Types"); } };
template<> struct Lookup<en_US, resource_browser> { static constexpr auto Value() { return FMT_STRING("Resource Browser"); } };
template<> struct Lookup<en_US, effect_editor> { static constexpr auto Value() { return FMT_STRING("Effect Editor"); } };
template<> struct Lookup<en_US, model_viewer> { static constexpr auto Value() { return FMT_STRING("Model Viewer"); } };
template<> struct Lookup<en_US, information_center> { static constexpr auto Value() { return FMT_STRING("Information Center"); } };
template<> struct Lookup<en_US, game_mode> { static constexpr auto Value() { return FMT_STRING("Game Mode"); } };
template<> struct Lookup<en_US, version> { static constexpr auto Value() { return FMT_STRING("Version"); } };

/* en_GB */
template<> struct Lookup<en_GB, color> { static constexpr auto Value() { return FMT_STRING("Colour"); } };

/* ja_JP */
template<> struct Lookup<ja_JP, color> { static constexpr auto Value() { return FMT_STRING("色"); } };
template<> struct Lookup<ja_JP, branch> { static constexpr auto Value() { return FMT_STRING("分派"); } };
template<> struct Lookup<ja_JP, commit> { static constexpr auto Value() { return FMT_STRING("預ける"); } };
template<> struct Lookup<ja_JP, date> { static constexpr auto Value() { return FMT_STRING("年月日"); } };
template<> struct Lookup<ja_JP, new_project> { static constexpr auto Value() { return FMT_STRING("新しいプロジェクト"); } };
template<> struct Lookup<ja_JP, open_project> { static constexpr auto Value() { return FMT_STRING("プロジェクトを開きます"); } };
template<> struct Lookup<ja_JP, extract_game> { static constexpr auto Value() { return FMT_STRING("ビデオゲームを抽出"); } };
template<> struct Lookup<ja_JP, name> { static constexpr auto Value() { return FMT_STRING("名"); } };
template<> struct Lookup<ja_JP, type> { static constexpr auto Value() { return FMT_STRING("タイプ"); } };
template<> struct Lookup<ja_JP, size> { static constexpr auto Value() { return FMT_STRING("サイズ"); } };
template<> struct Lookup<ja_JP, directory> { static constexpr auto Value() { return FMT_STRING("ディレクトリ"); } };
template<> struct Lookup<ja_JP, file> { static constexpr auto Value() { return FMT_STRING("ファイル"); } };
template<> struct Lookup<ja_JP, file_name> { static constexpr auto Value() { return FMT_STRING("ファイル名"); } };
template<> struct Lookup<ja_JP, cancel> { static constexpr auto Value() { return FMT_STRING("キャンセル"); } };
template<> struct Lookup<ja_JP, system_locations> { static constexpr auto Value() { return FMT_STRING("システムの場所"); } };
template<> struct Lookup<ja_JP, recent_projects> { static constexpr auto Value() { return FMT_STRING("最近使ったプロジェクト"); } };
template<> struct Lookup<ja_JP, recent_files> { static constexpr auto Value() { return FMT_STRING("最近使用したファイル"); } };

template <typename Action, typename... Args>
constexpr auto Do(ELocale l, Action act, Args&&... args) {
  switch (l) {
    default:
    case ELocale::en_US:
      return act.template Do<en_US>(std::forward<Args>(args)...);
    case ELocale::en_GB:
      return act.template Do<en_GB>(std::forward<Args>(args)...);
    case ELocale::ja_JP:
      return act.template Do<ja_JP>(std::forward<Args>(args)...);
  }
}
