#include <Arduino.h>

const char vowels[] = {'a', 'e', 'i', 'o', 'u'};
const char consonants[] = {'b', 'c', 'd', 'f', 'g', 'h', 'j', 'k', 'l', 'm', 'n', 'p', 'q', 'r', 's', 't', 'v', 'w', 'x', 'y', 'z'};
const int num_vowels = sizeof(vowels) / sizeof(vowels[0]);
const int num_consonants = sizeof(consonants) / sizeof(consonants[0]);

char generate_random_letter(bool is_vowel) {
	int random_value = random(is_vowel ? num_vowels : num_consonants);
	return is_vowel ? vowels[random_value] : consonants[random_value];
}

String generate_random_word(int length) {
	String result;
	for (int i = 0; i < length; ++i) {
		result += generate_random_letter(random(2) == 0);
	}
	return result;
}

String generate_random_sentence() {
	String result;
	int word_count = random(4, 10 + 1);
	for (int i = 0; i < word_count; ++i) {
		String word = generate_random_word(random(2, 10 + 1));
		result += (i == 0 ? char(toupper(word[0])) : word[0]) + word.substring(1) + " ";
	}
	result[result.length() - 1] = '.';
	return result;
}

String generate_random_string() {
	String result;
	for (int i = 0; i < 8; ++i) {
		result += generate_random_sentence() + " ";
	}
	return result;
}

