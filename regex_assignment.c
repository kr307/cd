/*
Assignment D3-2 : Regular Expressions (grep, sed, awk)

Contents of input.txt used:

aa
ccc
hello
world
lowercaseonly
123a
99apples
123-456-7890
user@example.com
invalid-email@
http://example.com
https://google.com
apple_z
a_long_string_ending_with_z
replace 123 with 456 digits
one two three
apple banana
aae
vowel_test_ee
abc123def
Regex is awesome
This line contains REGEX

==================================================

Question 1: Find all lines that contain consecutive repeating characters (e.g., aa, ccc, etc.)

Command:
grep -E '(.)\1' input.txt

Output:
aa
ccc
hello
99apples
http://example.com
https://google.com
apple_z
one two three
apple banana
aae
vowel_test_ee

==================================================

Question 2: Match all lines that contain only lowercase letters from start to end.

Command:
grep -E '^[a-z]+$' input.txt

Output:
aa
ccc
hello
world
lowercaseonly
aae

==================================================

Question 3: Find lines that contain a digit immediately followed by a letter.

Command:
grep -E '[0-9][a-zA-Z]' input.txt

Output:
123a
99apples
abc123def

==================================================

Question 4: Extract lines containing a phone number in the format 123-456-7890.

Command:
grep -E '^[0-9]{3}-[0-9]{3}-[0-9]{4}$' input.txt

Output:
123-456-7890

==================================================

Question 5: Find all lines that contain a valid email address.

Command:
grep -E '[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,}' input.txt

Output:
user@example.com

==================================================

Question 6: Match all valid URLs that start with http:// or https://.

Command:
grep -E '^https?://[a-zA-Z0-9.-]+' input.txt

Output:
http://example.com
https://google.com

==================================================

Question 7: Find lines that start with a and end with z.

Command:
grep -E '^a.*z$' input.txt

Output:
apple_z
a_long_string_ending_with_z

==================================================

Question 8: Using sed, replace all digits in the file with #.

Command:
sed 's/[0-9]/#/g' input.txt

Output:
aa
ccc
hello
world
lowercaseonly
###a
##apples
###-###-####
user@example.com
invalid-email@
http://example.com
https://google.com
apple_z
a_long_string_ending_with_z
replace ### with ### digits
one two three
apple banana
aae
vowel_test_ee
abc###def
Regex is awesome
This line contains REGEX

==================================================

Question 9: Print only those lines that contain exactly 3 words.

Command:
awk 'NF==3' input.txt

Output:
one two three
Regex is awesome

==================================================

Question 10: Find all lines where a vowel is repeated (e.g., aa, ee, etc.).

Command:
grep -E '([aeiou])\1' input.txt

Output:
aa
https://google.com
one two three
aae
vowel_test_ee

==================================================

Question 11: Match lines that contain both letters and numbers (alphanumeric combinations).

Command:
grep -E '[a-zA-Z]' input.txt | grep -E '[0-9]'

Output:
123a
99apples
replace 123 with 456 digits
abc123def

==================================================

Question 12: Count how many lines in the file mention the word "regex", case-insensitively.

Command:
grep -i -c "regex" input.txt

Output:
2
*/
