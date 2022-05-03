#ifndef EMAIL_SERVICE
#define EMAIL_SERVICE

#include <Arduino.h>
#include <EMailSender.h>

class EmailService {
private:
    EMailSender emailSender;

    const char** emailRecipients;
    size_t recipientsCount;
public:
    EmailService(const char* email, const char* emailPassword, const char** emailRecipients, int recipientsCount) :
        emailSender(email, emailPassword), emailRecipients(emailRecipients), recipientsCount(recipientsCount) {

    }

    void sendEmail(String title, String message) {
        EMailSender::EMailMessage msg;
        msg.subject = title;
        msg.message = message;

        EMailSender::Response response = emailSender.send(emailRecipients, recipientsCount, msg);
        Serial.println("Sending warning mails status: ");
        Serial.println("code: " + response.code);
        Serial.println("desc: " + response.desc);
    }
};


#endif