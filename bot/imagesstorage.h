#ifndef IMAGESSTORAGE_H
#define IMAGESSTORAGE_H

#include <mutex>
#include <vector>

#include <QImage>


class ImagesStorage
{
 public:
  ImagesStorage();
  virtual ~ImagesStorage();

  void SetImageState(char state, const QImage& image);
  bool GetStateByImage(const QImage& image, char& state);

 private:
  ImagesStorage(const ImagesStorage&) = delete;
  ImagesStorage(ImagesStorage&&) = delete;
  ImagesStorage& operator=(const ImagesStorage&) = delete;
  ImagesStorage& operator=(ImagesStorage&&) = delete;

  const size_t kMinimalNumber = 1;
  const size_t kMaximalSaveTry = 10;
  const int kCompressQuality = 100;

  const QString kStorageFolder = QString::fromUtf8(u8"images");
  struct ImageInfo {
    QImage image_;
    char state_;
  };

  std::vector<ImageInfo> images_;
  std::mutex images_lock_;

  struct StateInfo {
    char state_;
    QString folder_name_;

    StateInfo(char state, const QString& folder_name) {
      state_ = state;
      folder_name_ = folder_name;
    }
  };

  std::vector<StateInfo> states_;

  void Load();
  void SaveImage(char state, const QImage& image);
  bool GetStateByFolderName(const QString& folder_name, char& state);
  void GetFolderNameByState(char state, QString& folder_name);
};

#endif // IMAGESSTORAGE_H
