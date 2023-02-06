const fs = require("fs");

const file = process.argv[2];
const content = fs.readFileSync(`benchmark/${file}/template.txt`).toString();

const repeatRegexp = /\{\{repeat:(\d+)}}(.*?)\{\{end}}/s;
const repeatTemplate = content.match(repeatRegexp);
let repeat = '';
for (let i = 0; i < parseInt(repeatTemplate[1], 10); i++) {
  repeat += repeatTemplate[2].replace('{{n}}', i);
}

fs.writeFileSync(process.argv[3], content.replace(repeatRegexp, repeat));
