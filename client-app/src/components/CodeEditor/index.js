import React, { useRef } from "react";
import ReactDOM from "react-dom";
import Editor from "@monaco-editor/react";
import { initiateLanguage, initiateTheme } from "../../tools/Configurators";

export default function CodeEditor() {

  const monacoRef = useRef(null);
  const editorRef = useRef(null);

  function handleEditorWillMount(monaco) {
    initiateLanguage(monaco);
    initiateTheme(monaco);
    monacoRef.current = monaco;
  }

  function handleEditorDidMount(editor, monaco) {
    editorRef.current = editor;
  }
  
  function showValue() {
    alert(editorRef.current.getValue());
  }

  return (
   <>
     <Editor
       style={{borderRadius: 16}}
       width="100%"
       height="50vh"
       language="Elpis"
       defaultValue="// some comment"
       beforeMount={handleEditorWillMount}
       onMount={handleEditorDidMount}
       theme={'ElpisTheme'}
     />
   </>
  );
}
