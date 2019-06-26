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
echo -n \"login $namepass\" > $(cd \"$(dirname \"$0\")\";pwd)/../.run_command\n\
if [ $? -eq 0 ]; then\n\
    zenity --info --text=\"Login successful.\"\n\
else\n\
    zenity --error --text=\"Login failed.\"\n\
fi";

#endif // SCRIPT_H
