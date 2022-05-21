
import React, { useState, useRef } from "react";
import { createMuiTheme, ThemeProvider, withStyles } from '@material-ui/core/styles';
import Box from '@material-ui/core/Box';
import Button from '@material-ui/core/Button';
import Grid from '@material-ui/core/Grid';
import GridList from '@material-ui/core/GridList';
import GridListTile from '@material-ui/core/GridListTile';
import ButtonGroup from '@material-ui/core/ButtonGroup';
import Checkbox from '@material-ui/core/Checkbox';
import Fab from '@material-ui/core/Fab';
import { AppBar, Typography, BottomNavigation, BottomNavigationAction, List, ListItem, ListSubheader,
     ListItemText, ListItemSecondaryAction, Paper, Divider, TextField, InputLabel, Input, FormHelperText,
     FormControl, 
     FormControlLabel,
     Icon,
     InputBase,
     Select,
     MenuItem,
     FilledInput,
     OutlinedInput,
     TablePagination} from "@material-ui/core";
import { createGlobalStyle } from 'styled-components';
import EditIcon from '@material-ui/icons/Edit';
import ContactSupportIcon from '@material-ui/icons/ContactSupport';
import FolderIcon from '@material-ui/icons/Folder';
import RestoreIcon from '@material-ui/icons/Restore';
import FavoriteIcon from '@material-ui/icons/Favorite';
import LocationOnIcon from '@material-ui/icons/LocationOn';
import LocalHospitalIcon from '@material-ui/icons/LocalHospital';
import Avatar from '@material-ui/core/Avatar';
import { fadeIn, fadeInUp, bounce, fadeInUpBig, fadeInDownBig, fadeInRightBig,
         fadeInDown, bounceOutDown, bounceInUp, zoomIn, zoomOut } from 'react-animations'
import styled, { keyframes } from 'styled-components';
import DeleteIcon from '@material-ui/icons/Delete';
import IconButton from '@material-ui/core/IconButton';
import useScrollTrigger from '@material-ui/core/useScrollTrigger';
import Container from '@material-ui/core/Container';
import Slide from '@material-ui/core/Slide';
import PropTypes from 'prop-types';
import PersonIcon from '@material-ui/icons/Person';
import CloseIcon from '@material-ui/icons/Close';
import HideAppBar from './HideAppBar';
import fadeOutUpBig from "react-animations/lib/fade-out-up-big";
import fadeOutDownBig from "react-animations/lib/fade-out-down-big";
import AccountCircleIcon from '@material-ui/icons/AccountCircle';
import VpnKeyIcon from '@material-ui/icons/VpnKey';
import { create } from 'jss';
import rtl from 'jss-rtl';
import { StylesProvider, jssPreset } from '@material-ui/core/styles';
import ListAltIcon from '@material-ui/icons/ListAlt';
import DoneIcon from '@material-ui/icons/Done';
import ChatIcon from '@material-ui/icons/Chat';
import { makeStyles } from "@material-ui/core/styles";
import RssFeedIcon from '@material-ui/icons/RssFeed';
import {Button as FluentButton} from 'react-uwp/Button';
import {TransformCard} from 'react-uwp/TransformCard';
import InfoOutlinedIcon from '@material-ui/icons/InfoOutlined';
import Table from '@material-ui/core/Table';
import TableBody from '@material-ui/core/TableBody';
import TableCell from '@material-ui/core/TableCell';
import TableContainer from '@material-ui/core/TableContainer';
import TableHead from '@material-ui/core/TableHead';
import TableRow from '@material-ui/core/TableRow';
import SettingsIcon from '@material-ui/icons/Settings';
import NotificationsIcon from '@material-ui/icons/Notifications';
import HomeIcon from '@material-ui/icons/Home';
import AssignmentIcon from '@material-ui/icons/Assignment';
import SearchIcon from '@material-ui/icons/Search';
import PlaylistAddCheckIcon from '@material-ui/icons/PlaylistAddCheck';
import AddCircleOutlineIcon from '@material-ui/icons/AddCircleOutline';
import DescriptionIcon from '@material-ui/icons/Description';
import AddAPhotoIcon from '@material-ui/icons/AddAPhoto';
import MUIDataTable from "mui-datatables";
import AddIcon from '@material-ui/icons/Add';

// Configure JSS
const jss = create({ plugins: [...jssPreset().plugins, rtl()] });

function RTL(props) {
  return (
    <StylesProvider jss={jss}>
      {props.children}
    </StylesProvider>
  );
}

let Components = {
    'AddIcon': AddIcon,
    'TablePagination': TablePagination,
    'MUIDataTable': MUIDataTable,
    'AddAPhotoIcon': AddAPhotoIcon,
    'OutlinedInput': OutlinedInput,
    'FilledInput': FilledInput,
    'DescriptionIcon': DescriptionIcon,
    'AddCircleOutlineIcon': AddCircleOutlineIcon,
    'PlaylistAddCheckIcon': PlaylistAddCheckIcon,
    'PersonIcon': PersonIcon,
    'SearchIcon': SearchIcon,
    'AssignmentIcon': AssignmentIcon,
    'HomeIcon': HomeIcon,
    'NotificationsIcon': NotificationsIcon,
    'SettingsIcon': SettingsIcon,
    'Table': Table,
    'TableBody': TableBody,
    'TableCell': TableCell,
    'TableContainer': TableContainer,
    'TableHead': TableHead,
    'TableRow': TableRow,
    'Container': Container,
    'Button': Button,
    'Box': Box,
    'Checkbox': Checkbox,
    'ButtonGroup': ButtonGroup,
    'Fab': Fab,
    'Grid': Grid,
    'GridList': GridList,
    'GridListTile': GridListTile,
    'AppBar': AppBar,
    'Typography': Typography,
    'EditIcon': EditIcon,
    'ContactSupportIcon': ContactSupportIcon,
    'BottomNavigation': BottomNavigation,
    'BottomNavigationAction': BottomNavigationAction,
    'FolderIcon': FolderIcon,
    'RestoreIcon': RestoreIcon,
    'FavoriteIcon': FavoriteIcon,
    'LocationOnIcon': LocationOnIcon,
    'LocalHospitalIcon': LocalHospitalIcon,
    'Avatar': Avatar,
    'ListItem': ListItem,
    'List': List,
    'ListSubheader': ListSubheader,
    'ListItemText': ListItemText,
    'DeleteIcon': DeleteIcon,
    'ListItemSecondaryAction': ListItemSecondaryAction,
    'IconButton': IconButton,
    'Paper': Paper,
    'Divider': Divider,
    'CloseIcon': CloseIcon,
    'AccountCircleIcon': AccountCircleIcon,
    'TextField': TextField,
    'VpnKeyIcon': VpnKeyIcon,
    'InputLabel': InputLabel,
    'Input': Input,
    'FormHelperText': FormHelperText,
    'FormControl': FormControl,
    'FormControlLabel': FormControlLabel,
    'Icon': Icon,
    'ListAltIcon': ListAltIcon,
    'DoneIcon': DoneIcon,
    'ChatIcon': ChatIcon,
    'React.Fragment': React.Fragment,
    'InputBase': InputBase,
    'RssFeedIcon': RssFeedIcon,
    'Select': Select,
    'MenuItem': MenuItem,
    'Fluent.Button': FluentButton,
    'Fluent.TransformCard': TransformCard,
    'InfoOutlinedIcon': InfoOutlinedIcon
}

let animations = {
    'bounce': bounce,
    'fadeIn': fadeIn,
    'fadeInUp': fadeInUp,
    'fadeInUpBig': fadeInUpBig,
    'fadeInDownBig': fadeInDownBig,
    'fadeInDown': fadeInDown,
    'fadeInRightBig': fadeInRightBig,
    'fadeOutUpBig': fadeOutUpBig,
    'fadeOutDownBig': fadeOutDownBig,
    'bounceOutDown': bounceOutDown,
    'bounceInUp': bounceInUp,
    'zoomIn': zoomIn,
    'zoomOut': zoomOut
}

export let appInstance = null;

export function setAppInstance(instance) {
    appInstance = instance;
}

let objCounter = 0;
let objDict = {};
let colorsDict = {};

let hooksList = [];

function searchForHooks(control) {
    if (control.properties !== undefined) {
        for (let prop in control.properties) {
            if (prop === 'id') {
                idDict[control.properties[prop]] = control;
            }
            else if (prop === 'ref') {
                let value = control.properties[prop];
                if (value.startsWith('${') && value.endsWith('}')) {
                    let variableName = value.substring(2, value.length - 1);
                    window['refsDict'][variableName] = 'Empty';
                }
            }
            else if ((typeof control.properties[prop]) === 'string') {
                let value = control.properties[prop];
                if (value.startsWith('${') && value.endsWith('}')) {
                    let variableName = value.substring(2, value.length - 1);
                    if (variableName.trim().startsWith("("))
                        continue;
                    else {
                        hooksList.push(variableName);
                    }
                }
            }
        }
    }

    for (let child in control.children) {
        searchForHooks(control.children[child]);
    }
}

let state = null;
let setState = null;
let idDict = {};

export function AppBody(props) {
    window['refsDict'] = {};
    let stateObj = {};
    if (state !== null) stateObj = state;
    hooksList = [];
    searchForHooks(props.body);
    window['refsDict'] = useRef(window['refsDict']);
    hooksList.forEach(hook => {
        stateObj = {
            ...stateObj,
            [hook]: null
        };
    });
    [state, setState] = useState(stateObj);
    window['setState'] = setState;
    window['state'] = state;
    window['setStyle'] = (elId, property, value) => {
        idDict[elId].style[property] = value;
        document.getElementById(elId).style[property] = value;
    };
    window['scrollToTop'] = function() {
        window.document.body.scrollTop = 0;
        window.document.documentElement.scrollTop = 0;
    };
    return (
            <div style={{width: '100%', height: '100%'}}>
                {props.body === null ? null : <Render body={props.body}/>}
            </div>
        );
}

export function Render(props) {
    let control = props.body;
    let transferredControl = {};
    if (control.controlType === 'Text') return control.text;
    else if (control.controlType === 'br') return (<br/>);
    else if (control.controlType === undefined) return;
    if (control.properties !== undefined) {
        for (let prop in control.properties) {
            if ((typeof control.properties[prop]) === 'string') {
                let value = control.properties[prop];
                if (value.startsWith('${') && value.endsWith('}')) {
                    let variableName = value.substring(2, value.length - 1);
                    if (variableName.trim().startsWith("(")) {
                        control.properties[prop] = eval(variableName);
                    }
                }
            }
            else if ((typeof control.properties[prop]) === 'object' && !Array.isArray(control.properties[prop])) {
                if (control.elpisTag === undefined) {
                    control.elpisTag = objCounter + '';
                }
                objDict[control.elpisTag + '-' + prop] = Render({body: control.properties[prop]});
                objCounter++;
                control.properties[prop] = undefined;
            }
        }
    }
    if (control.style !== undefined) {
        for (let prop in control.style) {
            if ((typeof control.style[prop]) === 'string') {
                let value = control.style[prop];
                if (value.startsWith('${') && value.endsWith('}')) {
                    let variableName = value.substring(2, value.length - 1);
                    if (variableName.trim().startsWith("("))
                        control.style[prop] = eval(variableName);
                }
            }
        }
    }
    let color = undefined;
    if (control.controlId !== undefined) {
        color = colorsDict[control.controlId];
        control.properties.color = 'secondary';
    }
    else if (control.properties !== undefined && control.properties.color !== undefined) {
        color = control.properties.color;
        control.controlId = objCounter + '';
        colorsDict[objCounter + ''] = color;
        objCounter++;
        control.properties.color = 'secondary';
    }
    var MyComponent = Components[control.controlType];
    if (MyComponent === undefined) {
        MyComponent = control.controlType;
    }
    let GlobalStyle = undefined;
    if (control.properties !== undefined && control.properties.fontName !== undefined) {
        control.style = {
            ...control.style,
            fontFamily: control.properties.fontName
        };
        GlobalStyle = createGlobalStyle`
            @font-face {
                font-family: '${control.properties.fontName}';
                src: url('${control.properties.fontPath}')
            }`
    }
    for (let prop in control.properties) {
        if ((typeof control.properties[prop]) === 'string' && control.properties[prop].startsWith('$')) {
            if (prop === 'ref') {
                transferredControl[prop] = (el) => {window.refsDict.current[control.properties[prop].substring(2, control.properties[prop].length - 1)] = el;};
            } else {
                transferredControl[prop] = state[control.properties[prop].substring(2, control.properties[prop].length - 1)];
            }
        }
        else {
            transferredControl[prop] = control.properties[prop];
        }
    }
    let objects = {};
    if (control.elpisTag !== undefined) {
        for (let prop in objDict) {
            if (prop.startsWith(control.elpisTag + '-')) {
                objects[prop.substring((control.elpisTag + '-').length)] = objDict[prop];
            }
        }
    }
    let result = undefined;
    if (control.properties !== undefined && control.properties.animation !== undefined) {
        const animation = keyframes`${animations[transferredControl.animation]}`;
        const ContainerDiv = control.style != undefined && control.style.zIndex !== undefined ? styled.div`
            z-index: ${control.style.zIndex};
            position: static;
        ` : Box;
        const BouncyDiv = styled.div`
            animation: ${transferredControl.animationTime} forwards ${animation};
        `;
        if (control.properties['animation'].startsWith('${') && control.properties['animation'].endsWith('}')) {
            if (control.controlType === 'AppBar' && control.properties.hideOnScroll === true) {
                result = (<ContainerDiv><BouncyDiv id={control.properties.id + "-anim"}>
                 {(<HideAppBar style={control.style}
                    {...transferredControl}
                    ref={transferredControl.ref}
                    {...(control.elpisTag !== undefined && {...objects})}
                     childrenOfAppBar={
                        control.children !== undefined ? control.children.map((child) => {
                            return Render({body: child});
                        }) : undefined
                    }/>)}
               </BouncyDiv></ContainerDiv>)
            }
            else {
              result = (<ContainerDiv><BouncyDiv id={control.properties.id + "-anim"}>
                {<MyComponent
                    style={control.style}
                    {...transferredControl}
                    ref={transferredControl.ref}
                    
                    {...(control.elpisTag !== undefined && {...objects})}
                > {control.children !== undefined ? control.children.map((child) => {
                    return Render({body: child});
                }) : undefined}
                </MyComponent>}
              </BouncyDiv></ContainerDiv>);
            }
        }
        else {
            if (control.controlType === 'AppBar' && control.properties.hideOnScroll === true) {
                result = (<ContainerDiv><BouncyDiv id={control.properties.id + "-anim"}>
                    (<HideAppBar style={control.style}
                    {...transferredControl}
                    ref={transferredControl.ref}
                    {...(control.elpisTag !== undefined && {...objects})}
                     childrenOfAppBar={
                        control.children !== undefined ? control.children.map((child) => {
                            return Render({body: child});
                        }) : undefined
                    }/>)</BouncyDiv></ContainerDiv>);
            }
            else {
              result = (<ContainerDiv><BouncyDiv id={control.properties.id + "-anim"}>
                {<MyComponent
                    style={control.style}
                    {...transferredControl}
                    ref={transferredControl.ref}
                    
                    {...(control.elpisTag !== undefined && {...objects})}
                >{control.children !== undefined ? control.children.map((child) => {
                    return Render({body: child});
                }) : undefined}
                </MyComponent>}
              </BouncyDiv></ContainerDiv>);
            }
        }
    }
    else {
        if (control.controlType === 'AppBar' && control.properties.hideOnScroll === true) {
            result = (<HideAppBar style={control.style}
                {...transferredControl}
                ref={transferredControl.ref}
                {...(control.elpisTag !== undefined && {...objects})}
                 childrenOfAppBar={
                    control.children !== undefined ? control.children.map((child) => {
                        return Render({body: child});
                    }) : undefined
                }/>);
        }
        else {
            result = 
              (<MyComponent
                style={control.style}
                {...transferredControl}
                ref={transferredControl.ref}
                
                {...(control.elpisTag !== undefined && {...objects})}>
                {control.children !== undefined ? control.children.map((child) => {
                    return Render({body: child});
                }) : undefined}
                </MyComponent>);
        }
    }
    if ((color === undefined || color === 'secondary') && (control.properties === undefined || control.properties.placeholderColor === undefined || control.properties.fontName === undefined)) return result;
    let theme = null;
    theme = createMuiTheme({
        ...(control.properties.placeholderColor !== undefined && {
            overrides: {
                MuiInput: {
                  input: {
                    "&::placeholder": {
                      color: control.properties.placeholderColor
                    }
                  }
                }
            }
        }),
        ...(color !== undefined && color !== 'secondary' && {palette: {
            secondary: {
                main: color
            }
        }})
    });
    if (GlobalStyle === undefined) {
        result = <ThemeProvider theme={theme}>
                {result}
            </ThemeProvider>
    }
    else {
        result = <ThemeProvider theme={theme}>
                    <div>
                        <GlobalStyle/>
                        {result}
                    </div>
                </ThemeProvider>
    }
    return result;
}