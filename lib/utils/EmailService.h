#ifndef EMAIL_SERVICE
#define EMAIL_SERVICE

#include <Arduino.h>
#include <EMailSender.h>

class EmailService {
private:
    EMailSender emailSender;

    const char* emailRecipient;
public:
    EmailService(const char* email, const char* emailPassword, const char* emailRecipient) :
        emailSender(email, emailPassword), emailRecipient(emailRecipient) {

    }

    void sendEmail(String title, String message) {
        EMailSender::EMailMessage msg;
        msg.subject = title;
        msg.message = message;

        EMailSender::Response response = emailSender.send(&emailRecipient, 1, msg);
        Serial.println("Sending warning mails status: ");
        Serial.println("code: " + response.code);
        Serial.println("desc: " + response.desc);
    }
};


#endif