#ifndef OLED_SCREEN_H
#define OLED_SCREEN_H

void oled_setup();
void displayWiFiIcon_Connected();
void displayWiFiIcon_Disconnected();
void display_login_admin_menu();
void display_login_admin_fail();
void display_admin_menu_main();
void display_CLEAR();
void display_tag_your_key_card();
void display_DOOR_IDLE();
void display_DOOR_CHECKING();
void display_DOOR_CHECKING_GRANT();
void display_DOOR_CHECKING_DENIED();
void display_REQUEST_CARD();
void display_SUCCESS_ADD();
void display_ALREADY();
void display_DELETE_SUCESS();
void display_NOT_FOUND();
void display_ADMIN_LOGIN();
void display_AGAIN();
void display_REMOVE();
void display_HOLD_FINGER();
void display_please_close_door();
#endif