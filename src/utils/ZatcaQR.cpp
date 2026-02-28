#include "ZatcaQR.h"
#include "qrcodegen.hpp"
#include <QPainter>

using qrcodegen::QrCode;

void ZatcaQR::addTLV(QByteArray &result, int tag, const QByteArray &value) {
  result.append(static_cast<char>(tag));
  result.append(static_cast<char>(value.size()));
  result.append(value);
}

QByteArray ZatcaQR::generateTLV(const QString &sellerName,
                                const QString &vatNumber,
                                const QString &timestamp, double totalWithVat,
                                double vatAmount) {
  QByteArray tlv;
  addTLV(tlv, 1, sellerName.toUtf8());
  addTLV(tlv, 2, vatNumber.toUtf8());
  addTLV(tlv, 3, timestamp.toUtf8());
  addTLV(tlv, 4, QString::number(totalWithVat, 'f', 2).toUtf8());
  addTLV(tlv, 5, QString::number(vatAmount, 'f', 2).toUtf8());
  return tlv.toBase64();
}

QImage ZatcaQR::generateQRImage(const QByteArray &data, int moduleSize) {
  return encodeToQR(data, moduleSize);
}

QImage ZatcaQR::generateZatcaQR(const QString &sellerName,
                                const QString &vatNumber,
                                const QString &timestamp, double totalWithVat,
                                double vatAmount, int moduleSize) {
  QByteArray base64 =
      generateTLV(sellerName, vatNumber, timestamp, totalWithVat, vatAmount);
  return encodeToQR(base64, moduleSize);
}

QImage ZatcaQR::encodeToQR(const QByteArray &data, int moduleSize) {
  // Convert QByteArray to std::vector<uint8_t>
  std::vector<std::uint8_t> bytes(data.begin(), data.end());

  // Use the Nayuki QR Code generator (MIT license, ISO 18004 compliant)
  QrCode qr = QrCode::encodeBinary(bytes, QrCode::Ecc::MEDIUM);

  int qrSize = qr.getSize();
  int border = 4; // Quiet zone (required by spec)
  int imgSize = (qrSize + border * 2) * moduleSize;

  QImage img(imgSize, imgSize, QImage::Format_RGB32);
  img.fill(Qt::white);

  QPainter p(&img);
  p.setPen(Qt::NoPen);
  p.setBrush(Qt::black);

  for (int y = 0; y < qrSize; y++) {
    for (int x = 0; x < qrSize; x++) {
      if (qr.getModule(x, y)) {
        p.drawRect((x + border) * moduleSize, (y + border) * moduleSize,
                   moduleSize, moduleSize);
      }
    }
  }

  p.end();
  return img;
}
