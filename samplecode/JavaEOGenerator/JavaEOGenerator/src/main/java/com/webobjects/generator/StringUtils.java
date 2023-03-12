/*
 * Copyright 2005 - 2007  Apple, Inc. All rights reserved.
 *
 * IMPORTANT:  This Apple software is supplied to you by Apple, Inc. (“Apple”) in consideration of your agreement to the following terms, and your use, installation, modification or redistribution of this Apple software constitutes acceptance of these terms.  If you do not agree with these terms, please do not use, install, modify or redistribute this Apple software.
 *
 * In consideration of your agreement to abide by the following terms, and subject to these terms, Apple grants you a personal, non-exclusive license, under Apple’s copyrights in this original Apple software (the “Apple Software”), to use, reproduce, modify and redistribute the Apple Software, with or without modifications, in source and/or binary forms; provided that if you redistribute the Apple Software in its entirety and without modifications, you must retain this notice and the following text and disclaimers in all such redistributions of the Apple Software.  Neither the name, trademarks, service marks or logos of Apple Computer, Inc. may be used to endorse or promote products derived from the Apple Software without specific prior written permission from Apple.  Except as expressly stated in this notice, no other rights or licenses, express or implied, are granted by Apple herein, including but not limited to any patent rights that may be infringed by your derivative works or by other works in which the Apple Software may be incorporated.
 *
 * The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE OR IN COMBINATION WITH YOUR PRODUCTS.
 *
 * IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN  ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */
package com.webobjects.generator;

/**
 *
 */
public class StringUtils {

	/**
	 * Creates a pretty short class name for the name. This name is not a Java identifier. It inserts a space in front of every upper case character.
	 *
	 * @param camelString
	 *            the target string with upper case characters: StringWithCaps
	 * @return pretty name
	 */
	public static String toShortPrettyClassName(String camelString) {
		if (camelString == null) {
			return null;
		}
		String shortName;
		int dotIndex = camelString.lastIndexOf('.');
		if (dotIndex == -1) {
			shortName = camelString;
		} else {
			shortName = camelString.substring(dotIndex + 1);
		}
		if (shortName.startsWith("EO")) {
			shortName = shortName.substring(2);
		}
		StringBuffer nameBuffer = new StringBuffer(shortName);
		for (int i = 0; i < nameBuffer.length(); i++) {
			char ch = nameBuffer.charAt(i);
			if (Character.isUpperCase(ch) && i != 0) {
				nameBuffer.insert(i, ' ');
				i++;
			}
		}
		return nameBuffer.toString();
	}

	/**
	 * Return the index of the first letter in the name. This methods skips all non letter characters
	 *
	 * @param name
	 *            target string
	 * @return index of the first letter
	 */
	public static int firstLetterIndex(String name) {
		int letterIndex = -1;
		if (name != null) {
			int length = name.length();
			boolean found = false;
			for (int index = 0; !found && index < length; index++) {
				char ch = name.charAt(index);
				if (Character.isLetter(ch)) {
					letterIndex = index;
					found = true;
				} else if (ch != '_') {
					found = true;
				}
			}
		}
		return letterIndex;
	}

	/**
	 * Checks if the first letter of the name is an upper case character.
	 *
	 * @param name
	 *            target string
	 * @return true if the first letter of the name is an upper case character, false otherwise.
	 */
	public static boolean isUppercaseFirstLetter(String name) {
		int firstLetterIndex = StringUtils.firstLetterIndex(name);
		return (firstLetterIndex != -1 && Character.isUpperCase(name.charAt(firstLetterIndex)));
	}

	/**
	 * Checks if the first letter of the name is a lower case character.
	 *
	 * @param name
	 *            target string
	 * @return true if the first letter of the name is a lower case character, false otherwise.
	 */
	public static boolean isLowercaseFirstLetter(String name) {
		int firstLetterIndex = StringUtils.firstLetterIndex(name);
		return (firstLetterIndex != -1 && Character.isLowerCase(name.charAt(firstLetterIndex)));
	}

	/**
	 * Transform the first letter of the name to be a lower case character.
	 *
	 * @param name
	 *            target string
	 * @return transformed string
	 */
	public static String toLowercaseFirstLetter(String name) {
		int firstLetterIndex = StringUtils.firstLetterIndex(name);
		String newName;
		if (firstLetterIndex == -1) {
			newName = name;
		} else {
			StringBuffer sb = new StringBuffer();
			if (firstLetterIndex > 0) {
				sb.append(name.substring(0, firstLetterIndex));
			}
			sb.append(Character.toLowerCase(name.charAt(firstLetterIndex)));
			sb.append(name.substring(firstLetterIndex + 1));
			newName = sb.toString();
		}
		return newName;
	}

	/**
	 * Transform the first letter of the name to be an upper case character.
	 *
	 * @param name
	 *            target string
	 * @return transformed string
	 */
	public static String toUppercaseFirstLetter(String name) {
		int firstLetterIndex = StringUtils.firstLetterIndex(name);
		String newName;
		if (firstLetterIndex == -1) {
			newName = name;
		} else {
			StringBuffer sb = new StringBuffer();
			if (firstLetterIndex > 0) {
				sb.append(name.substring(0, firstLetterIndex));
			}
			sb.append(Character.toUpperCase(name.charAt(firstLetterIndex)));
			sb.append(name.substring(firstLetterIndex + 1));
			newName = sb.toString();
		}
		return newName;
	}

	/**
	 * Converts ThisIsATest to this_is_a_test
	 *
	 * @param camelString
	 *            the target string with upper case characters: StringWithCaps
	 * @return the string_with_underscores
	 */
	public static String camelCaseToUnderscore(String camelString) {
		StringBuffer underscore = new StringBuffer();
		boolean lastCharacterWasWordBreak = true;
		for (int i = 0; i < camelString.length(); i++) {
			char ch = camelString.charAt(i);
			if (Character.isUpperCase(ch) && !lastCharacterWasWordBreak) {
				underscore.append("_");
				lastCharacterWasWordBreak = true;
			} else {
				lastCharacterWasWordBreak = false;
			}
			underscore.append(Character.toLowerCase(ch));
		}
		return underscore.toString();
	}

	/**
	 * Transform name to the plural form of name. (Only works for English names)
	 *
	 * @param name
	 *            target name
	 * @return plural name
	 */
	public static String toPlural(String name) {
		String plural;
		if (name != null && name.length() > 0) {
			char ch = name.charAt(name.length() - 1);
			StringBuffer pluralBuffer = new StringBuffer(name);
			if (ch == 's' || ch == 'x') {
				pluralBuffer.append("es");
			} else if (ch == 'y') {
				pluralBuffer.setLength(pluralBuffer.length() - 1);
				pluralBuffer.append("ies");
			} else {
				pluralBuffer.append("s");
			}
			plural = pluralBuffer.toString();
		} else {
			plural = name;
		}
		return plural;
	}

	/**
	 * Checks if the path is a syntactically valid path
	 *
	 * @param path
	 *            target path
	 * @return true if the path is valid.
	 */
	public static boolean isKeyPath(String path) {
		return path != null && path.matches("^[^.][a-zA-Z0-9_.]+$");
	}
}
