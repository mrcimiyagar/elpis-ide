import { colors } from "../App";

export const initiateLanguage = (monaco) => {
  monaco.languages.register({ id: 'Elpis' });

  let keywords = [
    "use",
    "command",
    "times",
    "instance",
    "of",
    "define",
    "return",
    "prop",
    "on",
    "created",
    "class",
    "function",
    "with",
    "params",
    "for",
    "until",
    "if",
    "else",
    "then",
    "do",
    "true",
    "false",
    "not",
  ];
  let monarchTokenProvider = {
    keywords: keywords,

    typeKeywords: [],

    operators: [
      "=",
      ">",
      "<",
      "!",
      "~",
      "?",
      ":",
      "==",
      "<=",
      ">=",
      "!=",
      "++",
      "--",
      "+",
      "-",
      "*",
      "/",
      "^",
      "<<",
      ">>",
      ">>>",
      "+=",
      "-=",
      "*=",
      "/=",
      "&=",
      "|=",
      "^=",
      "mod=",
      "<<=",
      ">>=",
      ">>>=",
    ],

    // we include these common regular expressions
    symbols: /[=><!~?:&|+\-*\/\^%]+/,

    // C# style strings
    escapes:
      /\\(?:[abfnrtv\\"']|x[0-9A-Fa-f]{1,4}|u[0-9A-Fa-f]{4}|U[0-9A-Fa-f]{8})/,

    // The main tokenizer for our languages
    tokenizer: {
      root: [
        // strings
        [/"/, { token: "string.quote", bracket: "@open", next: "@string" }],

        [/define[\s]+command[\s]+{.*/, { token: "command" }],

        // identifiers and keywords
        [
          /[a-z_$][\w$]*/,
          {
            cases: {
              "@typeKeywords": "keyword",
              "@keywords": "keyword",
              "@default": "identifier",
            },
          },
        ],
        [/[A-Z][\w\$]*/, "type.identifier"], // to show class names nicely

        // whitespace
        { include: "@whitespace" },

        // delimiters and operators
        [/[{}()\[\]]/, "brackets"],
        [/[<>](?!@symbols)/, "brackets"],
        [/@symbols/, { cases: { "@operators": "operator", "@default": "" } }],

        // @ annotations.
        // As an example, we emit a debugging log message on these tokens.
        // Note: message are supressed during the first load -- change some lines to see them.
        [/@\s*[a-zA-Z_\$][\w\$]*/, { token: "annotation" }],

        // numbers
        [/\d*\.\d+([eE][\-+]?\d+)?/, "number.float"],
        [/0[xX][0-9a-fA-F]+/, "number.hex"],
        [/\d+/, "number"],

        // delimiter: after number because of .\d floats
        [/[;,.]/, "delimiter"],
      ],

      comment2: [
        [/[^\/*]+/, "comment2"],
        [/\/\*/, "comment2", "@push"],
        ["\\*/", "comment2", "@pop"],
        [/[\/*]/, "comment2"],
      ],

      string: [
        [/[^\\"]+/, "string"],
        [/@escapes/, "string.escape"],
        [/\\./, "string.escape.invalid"],
        [/"/, { token: "string.quote", bracket: "@close", next: "@pop" }],
      ],

      whitespace: [
        [/[ \t\r\n]+/, "white"],
        [/\/\*/, "comment2", "@comment2"],
        [/\/\/.*$/, "comment2"],
      ],
    },
  };

  monaco.languages.setMonarchTokensProvider("Elpis", monarchTokenProvider);

  monaco.languages.registerCompletionItemProvider("Elpis", {
    provideCompletionItems: () => {
      var suggestions = [
        {
          label: "ifelse",
          kind: monaco.languages.CompletionItemKind.Snippet,
          insertText: [
            "if ${1:condition} then do",
            "\t$0",
            "else do",
            "\t",
            "",
          ].join("\n"),
          insertTextRules:
            monaco.languages.CompletionItemInsertTextRule.InsertAsSnippet,
          documentation: "If-Else Statement",
        },
        {
          label: "ifelseif",
          kind: monaco.languages.CompletionItemKind.Snippet,
          insertText: [
            "if ${1:condition} then do",
            "\t$0",
            "else if ${2:condition} then do",
            "\t",
            "",
          ].join("\n"),
          insertTextRules:
            monaco.languages.CompletionItemInsertTextRule.InsertAsSnippet,
          documentation: "If-ElseIf Statement",
        },
        {
          label: "ifelseIfelse",
          kind: monaco.languages.CompletionItemKind.Snippet,
          insertText: [
            "if ${1:condition} then do",
            "\t$0",
            "else if ${2:condition} then do",
            "\t",
            "else do",
            "\t",
            "",
          ].join("\n"),
          insertTextRules:
            monaco.languages.CompletionItemInsertTextRule.InsertAsSnippet,
          documentation: "If-Else Statement",
        },
      ];
      return { suggestions: suggestions };
    },
  });
};

export const initiateTheme = (monaco) => {
  monaco.editor.defineTheme('ElpisTheme', {
    base: 'vs-dark',
    inherit: true,
    rules: [
      { token: 'keyword', foreground: 'FFA500', fontStyle: 'bold' },
      { token: 'operator', foreground: 'FFA500', fontStyle: 'bold' },
      { token: 'brackets', foreground: '00FFFF', fontStyle: 'bold' },
      { token: 'command', foreground: '0099FF', fontStyle: 'bold' },
      { token: 'comment2', foreground: '00FF99', fontStyle: 'bold' },
      { background: colors.colorDark2}
    ],
    colors: {
      'editor.background': colors.colorDark2,
      'editor.foreground': '#d9d7ce',
      'editorIndentGuide.background': '#393b41',
      'editorIndentGuide.activeBackground': '#494b51',
    },
  });
  
};