#include <cassert>

#include <QCoreApplication>
#include <QDir>

#include "imagesstorage.h"


using namespace std;

ImagesStorage::ImagesStorage() {
  states_.emplace_back(' ', QString::fromUtf8(u8"out"));
  states_.emplace_back('.', QString::fromUtf8(u8"close"));
  states_.emplace_back('*', QString::fromUtf8(u8"bombflag"));
  states_.emplace_back('0', QString::fromUtf8(u8"0"));
  states_.emplace_back('1', QString::fromUtf8(u8"1"));
  states_.emplace_back('2', QString::fromUtf8(u8"2"));
  states_.emplace_back('3', QString::fromUtf8(u8"3"));
  states_.emplace_back('4', QString::fromUtf8(u8"4"));
  states_.emplace_back('5', QString::fromUtf8(u8"5"));
  states_.emplace_back('6', QString::fromUtf8(u8"6"));
  states_.emplace_back('7', QString::fromUtf8(u8"7"));
  states_.emplace_back('8', QString::fromUtf8(u8"8"));
  states_.emplace_back('x', QString::fromUtf8(u8"death"));
  Load();
}

ImagesStorage::~ImagesStorage() {
}

void ImagesStorage::SetImageState(char state, const QImage& image) {
  // TODO
  lock_guard<mutex> locker(images_lock_);
  // Add to current images
  ImageInfo info;
  info.state_ = state;
  info.image_ = image;
  images_.push_back(info);
  SaveImage(state, image);
}

bool ImagesStorage::GetStateByImage(const QImage& image, char& state) {
  // TODO
  lock_guard<mutex> locker(images_lock_);
  for (auto& info: images_) {
    if (info.image_ == image) {
      state = info.state_;
      return true;
    }
  }
  return false;
}

void ImagesStorage::Load() {
  lock_guard<mutex> locker(images_lock_);
  images_.clear();
  QString storage_path = QCoreApplication::applicationDirPath() + QChar('/') + kStorageFolder;
  QDir storage_dir(storage_path);
  if (!storage_dir.exists()) {
    // Storage don't exist
    return;
  }
  auto subdir_list = storage_dir.entryList();
  for (auto iter = subdir_list.begin(); iter != subdir_list.end(); ++iter) {
    char state;
    QString state_path = storage_path + QChar('/') + *iter;
    if (!GetStateByFolderName(*iter, state)) {
      // Unknown name. Pass it
      continue;
    }
    QDir state_dir(state_path);
    if (!state_dir.exists()) {
      // Directory isn't exist or directory isn't a directory
      continue;
    }
    auto files_list = state_dir.entryList();
    for (auto iter = files_list.begin(); iter != files_list.end(); ++iter) {
      ImageInfo info;
      info.state_ = state;
      if (!info.image_.load(state_dir.path() + QChar('/') + *iter)) {
        // File isn't image
        continue;
      }
      images_.push_back(info);
    }
  }
}


void ImagesStorage::SaveImage(char state, const QImage& image) {
  // TODO
  // Save to files
  QString storage_path = QCoreApplication::applicationDirPath() + QChar('/') + kStorageFolder;
  QString folder_name;
  GetFolderNameByState(state, folder_name);
  QString state_path = storage_path + QChar('/') + folder_name;
  QDir storage_dir(storage_path);
  if (!storage_dir.mkpath(folder_name)) {
    // Can't create path
    // TODO inform about error
    return;
  }
  QDir state_dir(state_path);
  auto files_list = state_dir.entryList();
  unsigned int max_number = kMinimalNumber;
  for (auto iter = files_list.begin(); iter != files_list.end(); ++iter) {
    QFileInfo file_info(state_path + QChar('/') + *iter);
    unsigned int number = (unsigned int)(file_info.baseName().toULong());
    if (number > max_number) {
      max_number = number;
    }
  }
  ++max_number;
  // Try to save image
  bool saved = false;
  for (size_t counter = 0; counter < kMaximalSaveTry; ++counter, ++max_number) {
    auto name = QString("%1.bmp").arg(max_number);
    QString full_name = state_path + QChar('/') + name;
    if (image.save(full_name, nullptr, kCompressQuality)) {
      saved = true;
      break;
    }
  }
  if (!saved) {
    // TODO inform about error
    return;
  }
  // Image is saved to file
}

bool ImagesStorage::GetStateByFolderName(const QString& folder_name, char& state) {
  // TODO
  auto iter = find_if(states_.begin(), states_.end(), [folder_name](const StateInfo& arg) {
    return arg.folder_name_ == folder_name;
  });
  if (iter == states_.end()) {
    return false;
  }
  state = iter->state_;
  return true;
}

void ImagesStorage::GetFolderNameByState(char state, QString& folder_name) {
  auto iter = find_if(states_.begin(), states_.end(), [state](const StateInfo& arg) {
    return arg.state_ == state;
  });
  if (iter == states_.end()) {
    assert(false);
  }
  folder_name = iter->folder_name_;
}
