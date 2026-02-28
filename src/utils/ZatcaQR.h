#pragma once
#include <QByteArray>
#include <QImage>
#include <QString>

/**
 * ZATCA (Zakat, Tax and Customs Authority) QR Code Generator
 * Implements TLV encoding per GAZT e-invoicing Phase 1 standard
 *
 * TLV Tags:
 *   1 = Seller Name
 *   2 = VAT Registration Number
 *   3 = Timestamp (ISO 8601)
 *   4 = Invoice Total (with VAT)
 *   5 = VAT Amount
 */
class ZatcaQR {
public:
  /// Generate ZATCA TLV-encoded Base64 string
  static QByteArray generateTLV(const QString &sellerName,
                                const QString &vatNumber,
                                const QString &timestamp, double totalWithVat,
                                double vatAmount);

  /// Generate QR code image from data bytes
  static QImage generateQRImage(const QByteArray &data, int moduleSize = 4);

  /// Convenience: generate ZATCA QR image directly
  static QImage generateZatcaQR(const QString &sellerName,
                                const QString &vatNumber,
                                const QString &timestamp, double totalWithVat,
                                double vatAmount, int moduleSize = 4);

private:
  static void addTLV(QByteArray &result, int tag, const QByteArray &value);

  // Simple QR Code encoder (Version 1-6, Error Correction M)
  static QImage encodeToQR(const QByteArray &data, int moduleSize);
};
