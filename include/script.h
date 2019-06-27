#ifndef SCRIPT_H
#define SCRIPT_H

char sc_script_login[] = "#!/bin/bash\n\
if command -v zenity >/dev/null 2>&1; then\n\
    namepass=`zenity --password --title=\"login\" --username | sed 's/|/ /g'`\n\
else\n\
    echo \"Need package 'zenity'.\"\n\
    zenity --notification --text=\"Need package 'zenity'.\"\n\
    exit 1\n\
fi\n\
echo -n \"login $namepass\" 1> $(cd \"$(dirname \"$0\")\";pwd)/../.run_command 2>/dev/null\n\
if [ $? -eq 0 ]; then\n\
    zenity --info --text=\"Login successful.\"\n\
else\n\
    zenity --error --text=\"Login failed.\"\n\
fi\n";

char sc_script_useradd[] = "#!/bin/bash\n\
if command -v zenity >/dev/null 2>&1; then\n\
    username=`zenity --entry --text=\"用户名\"`\n\
else\n\
    echo \"Need package 'zenity'.\"\n\
    zenity --notification --text=\"Need package 'zenity'.\"\n\
    exit 1\n\
fi\n\
echo -n \"useradd $username\" 1> $(cd \"$(dirname \"$0\")\";pwd)/../.run_command 2>/dev/null\n\
if [ $? -eq 0 ]; then\n\
    zenity --info --text=\"Useradd successful.\"\n\
else\n\
    zenity --error --text=\"Useradd failed.\"\n\
fi\n";

char sc_script_userdel[] = "#!/bin/bash\n\
if command -v zenity >/dev/null 2>&1; then\n\
    username=`zenity --entry --text=\"用户名\"`\n\
else\n\
    echo \"Need package 'zenity'.\"\n\
    zenity --notification --text=\"Need package 'zenity'.\"\n\
    exit 1\n\
fi\n\
echo -n \"userdel $username\" 1> $(cd \"$(dirname \"$0\")\";pwd)/../.run_command 2>/dev/null\n\
if [ $? -eq 0 ]; then\n\
    zenity --info --text=\"Userdel successful.\"\n\
else\n\
    zenity --error --text=\"Userdel failed.\"\n\
fi\n";

char sc_script_passwd[] = "#!/bin/bash\n\
if command -v zenity >/dev/null 2>&1; then\n\
    namepass=`zenity --password --title=\"passwd\" --username | sed 's/|/ /g'`\n\
else\n\
    echo \"Need package 'zenity'.\"\n\
    zenity --notification --text=\"Need package 'zenity'.\"\n\
    exit 1\n\
fi\n\
echo -n \"passwd $namepass\" 1> $(cd \"$(dirname \"$0\")\";pwd)/../.run_command 2>/dev/null\n\
if [ $? -eq 0 ]; then\n\
    zenity --info --text=\"Passwd successful.\"\n\
else\n\
    zenity --error --text=\"Passwd failed.\"\n\
fi\n";

char sc_script_groupadd[] = "#!/bin/bash\n\
if command -v zenity >/dev/null 2>&1; then\n\
    groupname=`zenity --entry --text=\"用户组名\"`\n\
else\n\
    echo \"Need package 'zenity'.\"\n\
    zenity --notification --text=\"Need package 'zenity'.\"\n\
    exit 1\n\
fi\n\
echo -n \"groupadd $groupname\" 1> $(cd \"$(dirname \"$0\")\";pwd)/../.run_command 2>/dev/null\n\
if [ $? -eq 0 ]; then\n\
    zenity --info --text=\"Groupadd successful.\"\n\
else\n\
    zenity --error --text=\"Groupadd failed.\"\n\
fi\n";

char sc_script_groupdel[] = "#!/bin/bash\n\
if command -v zenity >/dev/null 2>&1; then\n\
    groupname=`zenity --entry --text=\"用户组名\"`\n\
else\n\
    echo \"Need package 'zenity'.\"\n\
    zenity --notification --text=\"Need package 'zenity'.\"\n\
    exit 1\n\
fi\n\
echo -n \"groupdel $groupname\" 1> $(cd \"$(dirname \"$0\")\";pwd)/../.run_command 2>/dev/null\n\
if [ $? -eq 0 ]; then\n\
    zenity --info --text=\"Groupdel successful.\"\n\
else\n\
    zenity --error --text=\"Groupdel failed.\"\n\
fi\n";

char sc_script_gpasswd[] = "#!/bin/bash\n\
if command -v zenity >/dev/null 2>&1; then\n\
    username=`zenity --entry --text=\"用户名\"`\n\
    groupname=`zenity --entry --text=\"用户组名\"`\n\
else\n\
    echo \"Need package 'zenity'.\"\n\
    zenity --notification --text=\"Need package 'zenity'.\"\n\
    exit 1\n\
fi\n\
echo -n \"gpasswd $username $groupname\" 1> $(cd \"$(dirname \"$0\")\";pwd)/../.run_command 2>/dev/null\n\
if [ $? -eq 0 ]; then\n\
    zenity --info --text=\"Gpasswd successful.\"\n\
else\n\
    zenity --error --text=\"Gpasswd failed.\"\n\
fi\n";

#endif // SCRIPT_H
